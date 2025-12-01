# AirPlay Free for Windows

This project allows you to stream your Windows system audio to AirPlay devices (Apple TV, HomePod, AirPlay speakers) without installing any virtual audio drivers. It uses WASAPI Loopback to capture audio.

## Prerequisites

1.  **Visual Studio 2019 or 2022**
    *   Workload: "Desktop development with C++"
    *   Component: "C++ CMake tools for Windows"
2.  **Git**

## Build Instructions

We use **vcpkg** for dependency management and **CMake** for the build system. Visual Studio has built-in support for both.

### Option 1: Using Visual Studio (Recommended)

1.  **Clone the repository**:
    ```powershell
    git clone https://github.com/Khogao/Airplay-Free.git
    cd Airplay-Free
    ```

2.  **Open in Visual Studio**:
    *   Open Visual Studio.
    *   Select **"Open a local folder"**.
    *   Navigate to the `windows_app` folder inside the cloned repository and select it.

3.  **Install Dependencies (vcpkg)**:
    *   Visual Studio should automatically detect `CMakeLists.txt`.
    *   However, you need to tell it to use vcpkg.
    *   If you don't have vcpkg installed globally, you can install it locally:
        ```powershell
        git clone https://github.com/microsoft/vcpkg.git
        .\vcpkg\bootstrap-vcpkg.bat
        ```
    *   In Visual Studio, go to **Project > CMake Settings**.
    *   Add the following to "CMake toolchain file":
        `C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake` (Replace with your actual path).

4.  **Build**:
    *   Select your configuration (e.g., `x64-Debug` or `x64-Release`).
    *   Click **Build > Build All**.
    *   Visual Studio will download and compile Poco and OpenSSL automatically (this may take a while the first time).

5.  **Run**:
    *   Select `AirPlayFree.exe` as the startup item.
    *   Click the green "Play" button.

### Option 2: Command Line

1.  **Install vcpkg**:
    ```powershell
    git clone https://github.com/microsoft/vcpkg.git
    .\vcpkg\bootstrap-vcpkg.bat
    ```

2.  **Configure and Build**:
    ```powershell
    cd windows_app
    mkdir build
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build . --config Release
    ```

## Usage

1.  Run the application.
2.  An icon will appear in your System Tray (near the clock).
3.  **Right-click** the icon to see available AirPlay devices.
4.  **Click** on a device name to connect/disconnect.
    *   **Checked**: Audio is streaming to this device.
    *   **Unchecked**: Audio is not streaming to this device.
5.  You can select multiple devices to stream to all of them simultaneously.

## Troubleshooting

*   **No devices found**: Ensure your PC and AirPlay devices are on the same Wi-Fi/Ethernet network. Check firewall settings (allow the app to communicate on private networks).
*   **Audio delay**: AirPlay has a natural delay (2 seconds). This is normal for AirPlay 1.
