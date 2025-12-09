#include "WASAPICapture.h"
#include <iostream>

// Macro for error checking
#define EXIT_ON_ERROR(hres) \
    if (FAILED(hres))       \
    {                       \
        goto Exit;          \
    }
#define SAFE_RELEASE(punk) \
    if ((punk) != NULL)    \
    {                      \
        (punk)->Release(); \
        (punk) = NULL;     \
    }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

WASAPICapture::WASAPICapture() : _running(false) {}

WASAPICapture::~WASAPICapture()
{
    Stop();
    if (_pwfx)
    {
        CoTaskMemFree(_pwfx);
        _pwfx = nullptr;
    }
    SAFE_RELEASE(_enumerator);
    SAFE_RELEASE(_device);
    SAFE_RELEASE(_audioClient);
    SAFE_RELEASE(_captureClient);
}

bool WASAPICapture::Initialize(AudioDataCallback callback)
{
    HRESULT hr;
    _callback = callback;

    hr = CoInitialize(NULL); // Ensure COM is initialized

    hr = CoCreateInstance(
        CLSID_MMDeviceEnumerator, NULL,
        CLSCTX_ALL, IID_IMMDeviceEnumerator,
        (void **)&_enumerator);
    EXIT_ON_ERROR(hr)

    // Get default audio render device (speakers)
    hr = _enumerator->GetDefaultAudioEndpoint(
        eRender, eConsole, &_device);
    EXIT_ON_ERROR(hr)

    hr = _device->Activate(
        IID_IAudioClient, CLSCTX_ALL,
        NULL, (void **)&_audioClient);
    EXIT_ON_ERROR(hr)

    hr = _audioClient->GetMixFormat(&_pwfx);
    EXIT_ON_ERROR(hr)

    // Initialize the audio client in LOOPBACK mode
    // This is the magic that captures "What U Hear"
    hr = _audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        0, 0, _pwfx, NULL);
    EXIT_ON_ERROR(hr)

    hr = _audioClient->GetService(
        IID_IAudioCaptureClient,
        (void **)&_captureClient);
    EXIT_ON_ERROR(hr)

    return true;

Exit:
    // Handle errors (log them in a real app)
    return false;
}

void WASAPICapture::Start()
{
    if (_running)
        return;
    _running = true;
    _audioClient->Start();
    _thread = std::thread(&WASAPICapture::CaptureThread, this);
}

void WASAPICapture::Stop()
{
    _running = false;
    if (_thread.joinable())
    {
        _thread.join();
    }
    if (_audioClient)
    {
        _audioClient->Stop();
    }
}

void WASAPICapture::CaptureThread()
{
    HRESULT hr;
    UINT32 packetLength = 0;
    UINT32 numFramesAvailable;
    BYTE *pData;
    DWORD flags;

    while (_running)
    {
        // Sleep for half the buffer duration to avoid spinning
        // In a real app, use event-driven buffering
        Sleep(10);

        hr = _captureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr))
            continue;

        while (packetLength != 0)
        {
            hr = _captureClient->GetBuffer(
                &pData,
                &numFramesAvailable,
                &flags,
                NULL,
                NULL);

            if (FAILED(hr))
                break;

            if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
            {
                // Silence - maybe send zeros or just skip
            }
            else
            {
                // Calculate size in bytes
                // _pwfx->nBlockAlign is bytes per frame (e.g., 4 bytes for 16-bit stereo)
                size_t bytes = numFramesAvailable * _pwfx->nBlockAlign;
                if (_callback)
                {
                    _callback(pData, bytes);
                }
            }

            hr = _captureClient->ReleaseBuffer(numFramesAvailable);
            if (FAILED(hr))
                break;

            hr = _captureClient->GetNextPacketSize(&packetLength);
        }
    }
}
