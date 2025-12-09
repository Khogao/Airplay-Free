#pragma once

#include <Windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <thread>
#include <atomic>
#include <functional>
#include <vector>

// Callback function type: receives raw PCM data and size in bytes
using AudioDataCallback = std::function<void(const BYTE *data, size_t size)>;

class WASAPICapture
{
public:
    WASAPICapture();
    ~WASAPICapture();

    bool Initialize(AudioDataCallback callback);
    void Start();
    void Stop();

    const WAVEFORMATEX *GetFormat() const { return _pwfx; }

private:
    void CaptureThread();

    IMMDeviceEnumerator *_enumerator = nullptr;
    IMMDevice *_device = nullptr;
    IAudioClient *_audioClient = nullptr;
    IAudioCaptureClient *_captureClient = nullptr;
    WAVEFORMATEX *_pwfx = nullptr;

    std::thread _thread;
    std::atomic<bool> _running;
    AudioDataCallback _callback;
};
