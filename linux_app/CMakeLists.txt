cmake_minimum_required(VERSION 3.10)
project(AirplayFreeLinux)

set(CMAKE_CXX_STANDARD 14)

# Find dependencies
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(PkgConfig REQUIRED)

# Poco (Manual find as pkg-config is missing on some systems)
find_library(POCO_FOUNDATION_LIB PocoFoundation REQUIRED)
find_library(POCO_NET_LIB PocoNet REQUIRED)
find_library(POCO_UTIL_LIB PocoUtil REQUIRED)
set(POCO_LIBRARIES ${POCO_FOUNDATION_LIB} ${POCO_NET_LIB} ${POCO_UTIL_LIB})
set(POCO_INCLUDE_DIRS /usr/include)

pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(PULSE REQUIRED libpulse-simple libpulse)
pkg_check_modules(AVAHI REQUIRED avahi-compat-libdns_sd)

# We might need to link against a system ALAC library or include the source if provided
# For now, assuming system library or user provides it. 
# If system library is not standard, we might need to add it manually.
# find_library(ALAC_LIB alac) 

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/../rsoutput/sdk
    ${CMAKE_SOURCE_DIR}/../rsoutput/src/core
    ${CMAKE_SOURCE_DIR}/../rsoutput/src/core/impl
    ${CMAKE_SOURCE_DIR}/../rsoutput/lib/alac
    ${POCO_INCLUDE_DIRS}
    ${OPENSSL_INCLUDE_DIRS}
    ${PULSE_INCLUDE_DIRS}
    ${AVAHI_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/Platform_Linux.cpp
    src/PulseAudioSource.h
    src/LinuxPlayer.h
    
    # Core rsoutput files (we need to compile these)
    # Note: We are excluding Windows-specific files and using our Linux shims
    ../rsoutput/src/core/impl/DeviceDiscovery.cpp
    ../rsoutput/src/core/impl/DeviceInfo.cpp
    ../rsoutput/src/core/impl/DeviceManager.cpp
    ../rsoutput/src/core/impl/DeviceUtils.cpp
    ../rsoutput/src/core/impl/Options.cpp
    ../rsoutput/src/core/impl/OutputBuffer.cpp
    ../rsoutput/src/core/impl/OutputComponent.cpp
    ../rsoutput/src/core/impl/OutputFormat.cpp
    ../rsoutput/src/core/impl/OutputMetadata.cpp
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
)

# Add executable
add_executable(airplay-free ${SOURCES})

# Link libraries
target_link_libraries(airplay-free
    Qt5::Widgets
    ${POCO_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${PULSE_LIBRARIES}
    ${AVAHI_LIBRARIES}
    pthread
    dl
)

# Definitions for Linux build
target_compile_definitions(airplay-free PRIVATE
    TARGET_OS_LINUX
    POCO_OS_FAMILY_UNIX
)
