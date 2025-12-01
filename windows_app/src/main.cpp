#include <Windows.h>
#include <shellapi.h>
#include <string>
#include <memory>
#include "WASAPICapture.h"
#include "WindowsPlayer.h"
#include "OutputComponent.h"
#include "OutputFormat.h"

// System Tray Constants
#define WM_TRAYICON (WM_USER + 1)
#define ID_TRAYICON 1
#define ID_MENU_EXIT 1001

// Global Variables
NOTIFYICONDATA nid;
std::unique_ptr<WASAPICapture> g_capture;
std::unique_ptr<WindowsPlayer> g_player;
std::unique_ptr<OutputComponent> g_output;

void InitBackend() {
    g_player = std::make_unique<WindowsPlayer>();
    g_output = std::make_unique<OutputComponent>(*g_player);
    g_capture = std::make_unique<WASAPICapture>();

    // Initialize WASAPI Capture
    bool res = g_capture->Initialize([](const BYTE* data, size_t size) {
        if (g_output) {
            // Feed data to AirPlay engine
            // Note: WASAPI usually gives float data, rsoutput might expect PCM 16-bit
            // In a real implementation, we need a Resampler/Converter here!
            // For now, we assume the engine can handle it or we pass it raw.
            g_output->write(data, size);
        }
    });

    if (res) {
        // Configure Output Format (Assuming 44.1kHz 16-bit Stereo for now)
        // In reality, we should match _pwfx from WASAPICapture
        OutputFormat fmt(44100, 16, 2);
        g_output->open(fmt);
        
        g_capture->Start();
    }
}

void CleanupBackend() {
    if (g_capture) g_capture->Stop();
    if (g_output) g_output->close();
    g_capture.reset();
    g_output.reset();
    g_player.reset();
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        // Add Tray Icon
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
        break;

    case WM_TRAYICON:
        if (lParam == WM_RBUTTONUP) {
            POINT pt;
            GetCursorPos(&pt);
            HMENU hMenu = CreatePopupMenu();
            AppendMenu(hMenu, MF_STRING, ID_MENU_EXIT, L"Exit");
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN, pt.x, pt.y, 0, hwnd, NULL);
            DestroyMenu(hMenu);
        }
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == ID_MENU_EXIT) {
            DestroyWindow(hwnd);
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

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int) {
    const wchar_t CLASS_NAME[] = L"AirPlayFreeWindow";
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
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    // Main message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
