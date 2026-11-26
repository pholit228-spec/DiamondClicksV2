cmake_minimum_required(VERSION 3.21)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

# Official Geode recommended way (docs.geode-sdk.org)
if ("${CMAKE_SYSTEM_NAME}" STREQUAL "iOS" OR IOS)
    set(CMAKE_OSX_ARCHITECTURES "arm64")
    # Prevent multi-sysroot mismatch on newer Xcode (device + simulator)
    unset(CMAKE_APPLE_ARCH_SYSROOTS CACHE)
    set(CMAKE_OSX_SYSROOT "iphoneos" CACHE STRING "" FORCE)
else()
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64")
endif()

project(DiamondClicks VERSION 1.0.0)

file(GLOB SOURCES CONFIGURE_DEPENDS src/*.cpp)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

if (NOT DEFINED ENV{GEODE_SDK})
    message(FATAL_ERROR "Unable to find Geode SDK! Please define GEODE_SDK environment variable to point to Geode")
else()
    message(STATUS "Found Geode: $ENV{GEODE_SDK}")
endif()

add_subdirectory($ENV{GEODE_SDK} ${CMAKE_CURRENT_BINARY_DIR}/geode)

setup_geode_mod(${PROJECT_NAME})
