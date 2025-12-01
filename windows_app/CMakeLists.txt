cmake_minimum_required(VERSION 3.15)

project(AirPlayFreeWindows LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies via vcpkg
find_package(Poco CONFIG REQUIRED Foundation Net Util)
find_package(OpenSSL REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/../rsoutput/sdk
    ${CMAKE_SOURCE_DIR}/../rsoutput/src/core
    ${CMAKE_SOURCE_DIR}/../rsoutput/src/core/impl
    ${CMAKE_SOURCE_DIR}/../rsoutput/lib/alac
)

# Source files
set(SOURCES
    src/main.cpp
    src/WASAPICapture.cpp
    
    # Core rsoutput files
    ../rsoutput/src/core/impl/DeviceDiscovery.cpp
    ../rsoutput/src/core/impl/DeviceInfo.cpp
    ../rsoutput/src/core/impl/DeviceManager.cpp
    ../rsoutput/src/core/impl/DeviceUtils.cpp
    ../rsoutput/src/core/impl/Options.cpp
    ../rsoutput/src/core/impl/OutputBuffer.cpp
    ../rsoutput/src/core/impl/OutputComponent.cpp
    ../rsoutput/src/core/impl/OutputFormat.cpp
    ../rsoutput/src/core/impl/OutputMetadata.cpp
    ../rsoutput/src/core/impl/Platform.cpp
    ../rsoutput/src/core/impl/Plugin.cpp
    ../rsoutput/src/core/impl/RemoteControl.cpp
    ../rsoutput/src/core/impl/ServiceDiscovery.cpp
    
    # RAOP
    ../rsoutput/src/core/impl/raop/NTPTimestamp.cpp
    ../rsoutput/src/core/impl/raop/PacketBuffer.cpp
    ../rsoutput/src/core/impl/raop/RAOPDevice.cpp
    ../rsoutput/src/core/impl/raop/RAOPEngine.cpp
    ../rsoutput/src/core/impl/raop/RTSPClient.cpp
    
    # ALAC Utilities
    ../rsoutput/lib/alac/ALACBitUtilities.c
    ../rsoutput/lib/alac/EndianPortable.c
    ../rsoutput/lib/alac/ag_dec.c
    ../rsoutput/lib/alac/ag_enc.c
    ../rsoutput/lib/alac/dp_enc.c
    ../rsoutput/lib/alac/matrix_enc.c
)

# Add executable (Windows GUI application)
add_executable(AirPlayFree WIN32 ${SOURCES})

# Link libraries
target_link_libraries(AirPlayFree PRIVATE
    Poco::Foundation
    Poco::Net
    Poco::Util
    OpenSSL::SSL
    OpenSSL::Crypto
    ws2_32
    iphlpapi
    shlwapi
    ole32
    mmdevapi
)

# Definitions
target_compile_definitions(AirPlayFree PRIVATE
    TARGET_OS_WIN32
    _WIN32
    UNICODE
    _UNICODE
    POCO_STATIC
)
