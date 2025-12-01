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

## 3. Application Workflow (User Option 2: Loopback & Multi-Output)

This application will run in the System Tray and operate on a "Capture & Forward" model.

### A. The "Virtual" Logic
Since we are not writing a kernel driver, the app will not appear in Windows Sound Settings. Instead, it works like this:
1.  **Source:** The app silently captures whatever is playing on the user's *current default speaker* (e.g., Realtek Audio).
2.  **Routing:** The app provides a menu to decide where that audio goes *next*.

### B. User Interface (System Tray)
*   **Source:** [Auto-detect Default Output] (User can override if they have multiple sound cards).
*   **Destinations (Checkable Menu Items):**
    *   [x] **Local Speakers** (Default: Always on unless muted by user).
    *   [ ] **Living Room TV** (AirPlay).
    *   [ ] **Bedroom Speaker** (AirPlay).
    *   [x] **Kitchen HomePod** (AirPlay).

### C. Multi-Room Logic
*   **One-to-Many Streaming:** The captured PCM data is encoded once to ALAC, then sent via UDP to *all* selected AirPlay IP addresses simultaneously.
*   **Synchronization:** The app must send NTP timing packets to all devices to ensure they play in sync (crucial when multiple AirPlay speakers are active).

## 4. Recommended Tech Stack for Windows App

*   **Language:** C# (.NET 6/8) is recommended for rapid GUI and networking development.
*   **Audio Capture:** `NAudio` (library) -> `WasapiLoopbackCapture`.
*   **Discovery:** `Zeroconf` (NuGet package) or `Bonjour SDK`.
*   **Encoding:** `FFmpeg.AutoGen` or a C# wrapper for `libalac`.

## 5. Development Roadmap
1.  **Prototype Capture:** Build a console app that captures WASAPI loopback and saves to a .wav file (verify input).
2.  **Prototype Discovery:** Build a console app that lists all `_raop._tcp` services on the network.
3.  **Prototype Streaming:** Connect Capture -> Encode -> Network Socket -> Single AirPlay Device.
4.  **GUI Integration:** Wrap it all in a System Tray app with the multi-select menu.
