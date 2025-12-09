#include <Windows.h>
#include <shellapi.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <mutex>
#include <samplerate.h>

#include "WASAPICapture.h"
#include "WindowsPlayer.h"
#include "OutputComponent.h"
#include "OutputFormat.h"
#include "DeviceDiscovery.h"
#include "DeviceInfo.h"
#include "Options.h"
#include "DeviceNotification.h"

// System Tray Constants
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1
#define ID_MENU_EXIT 1001
#define ID_MENU_DEVICES_START 2000

// Global Variables
NOTIFYICONDATA nid;
std::unique_ptr<WASAPICapture> g_capture;
std::unique_ptr<WindowsPlayer> g_player;
std::unique_ptr<OutputComponent> g_output;

class AudioConverter
{
public:
    AudioConverter(int inputRate, int outputRate)
        : _srcState(nullptr), _ratio((double)outputRate / inputRate)
    {
        int error;
        _srcState = src_new(SRC_SINC_FASTEST, 2, &error);
    }

    ~AudioConverter()
    {
        if (_srcState)
            src_delete(_srcState);
    }

    std::vector<int16_t> Process(const float *input, size_t frames)
    {
        if (!_srcState)
            return {};

        // Prepare output buffer (approx size + margin)
        size_t outFrames = (size_t)(frames * _ratio) + 256;
        if (_floatBuffer.size() < outFrames * 2)
        {
            _floatBuffer.resize(outFrames * 2);
        }

        SRC_DATA data;
        data.data_in = input;
        data.data_out = _floatBuffer.data();
        data.input_frames = (long)frames;
        data.output_frames = (long)outFrames;
        data.src_ratio = _ratio;
        data.end_of_input = 0;

        int error = src_process(_srcState, &data);
        if (error != 0)
            return {};

        // Convert float to int16
        std::vector<int16_t> output(data.output_frames_gen * 2);
        src_float_to_short_array(data.data_out, output.data(), data.output_frames_gen * 2);

        return output;
    }

private:
    SRC_STATE *_srcState;
    double _ratio;
    std::vector<float> _floatBuffer;
};

std::unique_ptr<AudioConverter> g_converter;

// Device Management
class TrayDeviceListener : public DeviceDiscovery::Listener
{
public:
    void onDeviceFound(const DeviceInfo &info) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        auto it = devices.find(info.name());
        if (it != devices.end())
        {
            it->second = info;
        }
        else
        {
            devices.emplace(info.name(), info);
        }
        needsUpdate = true;
    }

    void onDeviceLost(const DeviceInfo &info) override
    {
        std::lock_guard<std::mutex> lock(mutex);
        devices.erase(info.name());
        needsUpdate = true;
    }

    std::map<std::string, DeviceInfo> getDevices()
    {
        std::lock_guard<std::mutex> lock(mutex);
        return devices;
    }

    bool checkAndUpdate()
    {
        bool ret = needsUpdate;
        needsUpdate = false;
        return ret;
    }

private:
    std::mutex mutex;
    std::map<std::string, DeviceInfo> devices;
    bool needsUpdate = false;
};

TrayDeviceListener g_listener;

// Defined in compat/dns_sd_stub.cpp
extern bool EnsureBonjourLoaded();

void InitBackend()
{
    try
    {
        if (!EnsureBonjourLoaded())
        {
            MessageBoxA(NULL, "Bonjour Service (dnssd.dll) not found!\nPlease install Bonjour Print Services for Windows.", "Error", MB_ICONERROR);
            return;
        }

        g_player = std::make_unique<WindowsPlayer>();
        g_output = std::make_unique<OutputComponent>(*g_player);
        g_capture = std::make_unique<WASAPICapture>();

        // Enable Activation Bypass
        Options::getOptions()->setBypassActivation(true);

        // Start Discovery
        DeviceDiscovery::browseDevices(g_listener);

        // Initialize WASAPI Capture
        bool res = g_capture->Initialize([](const BYTE *data, size_t size)
                                         {
            if (g_output && g_converter) {
                // WASAPI Loopback is typically 32-bit Float Stereo
                // We assume the input is float here based on typical Windows behavior
                // A more robust app would check wfx->wBitsPerSample in the callback or setup a specific converter
                
                size_t frames = size / (sizeof(float) * 2);
                auto converted = g_converter->Process((const float*)data, frames);
                
                if (!converted.empty()) {
                    g_output->write((const BYTE*)converted.data(), converted.size() * sizeof(int16_t));
                }
            } });

        if (res)
        {
            const WAVEFORMATEX *wfx = g_capture->GetFormat();
            int inputRate = wfx->nSamplesPerSec;

            // Initialize converter: Input Rate -> 44100
            g_converter = std::make_unique<AudioConverter>(inputRate, 44100);

            OutputFormat fmt(SampleRate(44100), SampleSize(16), ChannelCount(2));
            g_output->open(fmt);
            g_capture->Start();
        }
    }
    catch (const std::exception &e)
    {
        MessageBoxA(NULL, e.what(), "Initialization Error", MB_ICONERROR);
    }
    catch (...)
    {
        MessageBoxA(NULL, "Unknown error occurred during initialization.", "Initialization Error", MB_ICONERROR);
    }
}

void CleanupBackend()
{
    DeviceDiscovery::stopBrowsing(g_listener);
    if (g_capture)
        g_capture->Stop();
    if (g_output)
        g_output->close();
    g_capture.reset();
    g_output.reset();
    g_player.reset();
}

void UpdateTrayMenu(HWND hwnd)
{
    // This function would be called to refresh the menu
    // Since Win32 menus are usually created on demand in WM_TRAYICON,
    // we just ensure we have the latest data there.
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        memset(&nid, 0, sizeof(nid));
        nid.cbSize = sizeof(nid);
        nid.hWnd = hwnd;
        nid.uID = ID_TRAYICON;
        nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
        nid.uCallbackMessage = WM_TRAYICON;
        nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wcscpy_s(nid.szTip, L"AirPlay Free (Windows)");
        Shell_NotifyIcon(NIM_ADD, &nid);

        InitBackend();
        SetTimer(hwnd, 1, 1000, NULL); // Check for device updates every second
        break;

    case WM_TIMER:
        if (g_listener.checkAndUpdate())
        {
            // Optional: Show a balloon tip that devices changed
        }
        break;

    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP)
        {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();

            // Add Devices
            AppendMenu(hMenu, MF_STRING | MF_DISABLED, 0, L"Available Devices:");
            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

            auto devices = g_listener.getDevices();
            int id = ID_MENU_DEVICES_START;
            if (devices.empty())
            {
                AppendMenu(hMenu, MF_STRING | MF_GRAYED, 0, L"(Scanning...)");
            }
            else
            {
                auto options = Options::getOptions();
                for (const auto &pair : devices)
                {
                    std::wstring wname;
                    for (char c : pair.second.name())
                        wname += (wchar_t)c;

                    UINT flags = MF_STRING;
                    if (options->isActivated(pair.second.name()))
                    {
                        flags |= MF_CHECKED;
                    }

                    AppendMenu(hMenu, flags, id++, wname.c_str());
                }
            }

            AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
            AppendMenu(hMenu, MF_STRING, ID_MENU_EXIT, L"Exit");

            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_MENU_EXIT)
        {
            DestroyWindow(hwnd);
        }
        else if (LOWORD(wParam) >= ID_MENU_DEVICES_START)
        {
            int devIndex = LOWORD(wParam) - ID_MENU_DEVICES_START;
            auto devices = g_listener.getDevices();
            if (devIndex < devices.size())
            {
                auto it = devices.begin();
                std::advance(it, devIndex);
                const DeviceInfo &info = it->second;

                auto options = Options::getOptions();
                bool isActive = options->isActivated(info.name());
                options->setActivated(info.name(), !isActive);

                // Notify DeviceManager to open/close device
                Options::postNotification(new DeviceNotification(
                    isActive ? DeviceNotification::DEACTIVATE : DeviceNotification::ACTIVATE,
                    info));
            }
        }
        break;

    case WM_DESTROY:
        CleanupBackend();
        Shell_NotifyIcon(NIM_DELETE, &nid);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int)
{
    // Debug: Confirm app is running
    MessageBox(NULL, L"AirPlay Free is starting...", L"Debug", MB_OK);

    const wchar_t CLASS_NAME[] = L"AirPlayFreeTrayWindow";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    // Create a hidden window for message handling
    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"AirPlay Free",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
        return 0;

    // Main message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
