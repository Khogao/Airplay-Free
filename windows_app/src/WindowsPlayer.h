#pragma once
#include "Player.h"
#include <Windows.h>

// A dummy player implementation required by the rsoutput library.
// Since we are a standalone app capturing system audio, we don't control playback.
class WindowsPlayer : public Player {
public:
    WindowsPlayer() {}
    virtual ~WindowsPlayer() {}

    // Return NULL or a hidden window handle if needed
    virtual HWND window() const override { return NULL; }

    // No-ops for playback control
    virtual void play() override {}
    virtual void pause() override {}
    virtual void stop() override {}
    virtual void restart() override {}
    virtual void startNext() override {}
    virtual void startPrev() override {}
    virtual void increaseVolume() override {}
    virtual void decreaseVolume() override {}
    virtual void toggleMute() override {}
    virtual void toggleShuffle() override {}
};
