#ifndef LINUX_PLAYER_H
#define LINUX_PLAYER_H

#include "Player.h"
#include <iostream>

// Implementation of the Player interface required by rsoutput
class LinuxPlayer : public Player {
public:
    LinuxPlayer() {}
    virtual ~LinuxPlayer() {}

    // On Linux/Qt, we don't use HWND, so return NULL or a dummy handle
    virtual HWND window() const override { return NULL; }

    // Implement dummy controls since we are just a system audio streamer
    // We don't control the actual music player (Spotify, etc.)
    virtual void play() override { std::cout << "Player: Play" << std::endl; }
    virtual void pause() override { std::cout << "Player: Pause" << std::endl; }
    virtual void stop() override { std::cout << "Player: Stop" << std::endl; }
    virtual void restart() override { std::cout << "Player: Restart" << std::endl; }
    virtual void startNext() override { std::cout << "Player: Next" << std::endl; }
    virtual void startPrev() override { std::cout << "Player: Prev" << std::endl; }
    virtual void increaseVolume() override { std::cout << "Player: Vol+" << std::endl; }
    virtual void decreaseVolume() override { std::cout << "Player: Vol-" << std::endl; }
    virtual void toggleMute() override { std::cout << "Player: Mute" << std::endl; }
    virtual void toggleShuffle() override { std::cout << "Player: Shuffle" << std::endl; }
};

#endif // LINUX_PLAYER_H
