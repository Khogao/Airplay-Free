#ifndef PULSE_AUDIO_SOURCE_H
#define PULSE_AUDIO_SOURCE_H

#include <pulse/simple.h>
#include <pulse/error.h>
#include <thread>
#include <atomic>
#include <vector>
#include <functional>

class PulseAudioSource {
public:
    using DataCallback = std::function<void(const uint8_t* data, size_t size)>;

    PulseAudioSource() : s(nullptr), running(false) {}
    
    ~PulseAudioSource() {
        stop();
    }

    bool start(DataCallback callback) {
        if (running) return true;
        
        this->callback = callback;
        
        // Audio format specification
        static const pa_sample_spec ss = {
            .format = PA_SAMPLE_S16LE,
            .rate = 44100,
            .channels = 2
        };

        int error;
        // Create a recording stream that monitors the default sink (output)
        // "monitor" source of the default sink is usually what we want
        s = pa_simple_new(NULL,               // Use the default server
                          "AirplayFree",      // Our application name
                          PA_STREAM_RECORD,
                          NULL,               // Use the default device
                          "System Audio",     // Description of our stream
                          &ss,                // Our sample format
                          NULL,               // Use default channel map
                          NULL,               // Use default buffering attributes
                          &error              // Ignore error code
                          );

        if (!s) {
            // fprintf(stderr, __FILE__": pa_simple_new() failed: %s\n", pa_strerror(error));
            return false;
        }

        running = true;
        captureThread = std::thread(&PulseAudioSource::captureLoop, this);
        return true;
    }

    void stop() {
        running = false;
        if (captureThread.joinable()) {
            captureThread.join();
        }
        if (s) {
            pa_simple_free(s);
            s = nullptr;
        }
    }

private:
    pa_simple *s;
    std::thread captureThread;
    std::atomic<bool> running;
    DataCallback callback;

    void captureLoop() {
        const size_t bufferSize = 4096;
        std::vector<uint8_t> buffer(bufferSize);
        int error;

        while (running) {
            if (pa_simple_read(s, buffer.data(), bufferSize, &error) < 0) {
                // fprintf(stderr, __FILE__": pa_simple_read() failed: %s\n", pa_strerror(error));
                break; // Exit loop on error
            }
            
            if (callback) {
                callback(buffer.data(), bufferSize);
            }
        }
    }
};

#endif // PULSE_AUDIO_SOURCE_H
