# Windows AirPlay Sender Implementation Plan

This document outlines the technical architecture for building a native Windows application that streams system audio to AirPlay devices (AirPort Express, Apple TV, etc.).

## 1. Core Architecture

Unlike Linux (which uses PipeWire) or macOS (CoreAudio), Windows does not have a built-in AirPlay sender. To achieve this, we must implement three main components:

1.  **Audio Capture (Input):** Getting the system audio.
2.  **Audio Encoding (Processing):** Converting raw audio to AirPlay-compatible format (ALAC).
3.  **Network Streaming (Output):** Sending the data via RTSP/RAOP protocol.

## 2. Technical Implementation Details

### A. Audio Capture: WASAPI Loopback
Instead of using virtual audio drivers (like VB-Cable), the modern and "clean" way to capture audio on Windows (Vista+) is **WASAPI (Windows Audio Session API) Loopback Mode**.

*   **API:** `IAudioClient::Initialize` with `AUDCLNT_STREAMFLAGS_LOOPBACK`.
*   **Advantage:** Captures the exact digital stream going to the speakers (bit-perfect), extremely low latency, no driver installation required.
*   **Format:** Typically receives PCM Float 32-bit, 48kHz or 44.1kHz (depending on system settings).

### B. Audio Encoding: ALAC (Apple Lossless)
AirPlay (specifically AirTunes/RAOP) requires audio to be encrypted and encoded in ALAC format.

*   **Input:** PCM (from WASAPI).
*   **Output:** ALAC frames (44.1kHz, 16-bit, Stereo).
*   **Library:** Use `libalac` (open source) or FFmpeg libraries.
*   **Encryption:** AirPlay uses AES encryption with a session key exchanged during the handshake. You will need `OpenSSL` to handle the RSA and AES encryption.

### C. Network Protocol: RTSP & RAOP
This is the most complex part. You need to implement a client that speaks the Remote Audio Output Protocol.

1.  **Discovery:** Use **Bonjour (mDNS)** to find devices (`_raop._tcp`).
2.  **Handshake (RTSP):**
    *   `ANNOUNCE`: Tell the device what we are sending.
    *   `SETUP`: Negotiate ports (Control, Timing, Audio).
    *   `RECORD`: Start the stream.
3.  **Streaming (RTP):** Send the ALAC packets over UDP to the negotiated ports.
4.  **Timing:** Send NTP-like timing packets to ensure synchronization (crucial for multi-room audio).

## 3. Recommended Tech Stack for Windows App

*   **Language:** C++ (for WASAPI and performance) or C# (.NET 6+ with `CoreAudio` wrappers).
*   **GUI:** WinUI 3 or WPF (modern Windows look).
*   **Libraries:**
    *   `NAudio` (if using C#) or `RtAudio` (C++) for WASAPI capture.
    *   `mdns` or `Bonjour SDK` for discovery.
    *   `openssl` for encryption.

## 4. Existing Open Source References
Before building from scratch, examine these projects:
*   **WinStream:** A C# implementation of this exact architecture.
*   **Shairport Sync:** (Linux receiver) - good for understanding the protocol reverse-engineering.
*   **AirPlay-Protocol:** Various docs on GitHub documenting the RTSP headers.

## 5. Summary
To build this app:
1.  Capture audio with **WASAPI Loopback**.
2.  Encode to **ALAC**.
3.  Wrap in **RTSP/RTP** and send to the device found via **Bonjour**.
