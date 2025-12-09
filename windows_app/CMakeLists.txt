cmake_minimum_required(VERSION 3.15)

project(AirPlayFreeWindows LANGUAGES CXX)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies via vcpkg
find_package(Poco CONFIG REQUIRED Foundation Net Util)
find_package(OpenSSL REQUIRED)
find_package(SampleRate CONFIG REQUIRED)
find_library(ALAC_LIB NAMES alac libalac REQUIRED)
find_path(ALAC_INCLUDE_DIR NAMES alac/ALACEncoder.h REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/compat
    ${CMAKE_SOURCE_DIR}/../rsoutput/sdk
    ${CMAKE_SOURCE_DIR}/../rsoutput/src/core
    ${CMAKE_SOURCE_DIR}/../rsoutput/src/core/impl
    ${ALAC_INCLUDE_DIR}
    ${ALAC_INCLUDE_DIR}/alac
)

# Source files
set(SOURCES
    src/main.cpp
    src/WASAPICapture.cpp
    src/compat/dns_sd_stub.cpp
    
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
    ../rsoutput/src/core/impl/Debugger.cpp
    ../rsoutput/src/core/impl/OutputReformatter.cpp
    
    # RAOP
    ../rsoutput/src/core/impl/raop/NTPTimestamp.cpp
    ../rsoutput/src/core/impl/raop/PacketBuffer.cpp
    ../rsoutput/src/core/impl/raop/RAOPDevice.cpp
    ../rsoutput/src/core/impl/raop/RAOPEngine.cpp
    ../rsoutput/src/core/impl/raop/RTSPClient.cpp
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
    SampleRate::samplerate
    ${ALAC_LIB}
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
    WIN32_LEAN_AND_MEAN
    NOMINMAX
    RSOUTPUT_EXPORTS
)
