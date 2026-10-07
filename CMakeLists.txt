cmake_minimum_required(VERSION 3.24)
project(bbport LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

find_package(Vulkan REQUIRED)

# Global include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

# -----------------------------------------------------------------------------
# Third-party decoders: LibAtrac9 (C99, no -Werror)
# -----------------------------------------------------------------------------
file(GLOB ATRAC9_SOURCES "third_party/LibAtrac9/C/src/*.c")
if (ATRAC9_SOURCES)
    add_library(atrac9 STATIC ${ATRAC9_SOURCES})
    target_include_directories(atrac9 PUBLIC third_party/LibAtrac9/C/src)
    if (MSVC)
        target_compile_options(atrac9 PRIVATE /W0)
    else()
        target_compile_options(atrac9 PRIVATE -w -std=c99 -O2)
    endif()
endif()

# -----------------------------------------------------------------------------
# GPU Video Core Library (shadPS4 Vulkan Core + Two-stage DrawPipe + Upscalers)
# -----------------------------------------------------------------------------
add_subdirectory(gpu)

# -----------------------------------------------------------------------------
# Main Executable: bb-probe (Loader + HLE Runtime)
# -----------------------------------------------------------------------------
file(GLOB LOADER_SOURCES "src/loader/*.c")
file(GLOB RUNTIME_SOURCES "src/runtime*.c")
file(GLOB_RECURSE RUNTIME_SUB_SOURCES "src/runtime/*.c")
if (WIN32)
    file(GLOB_RECURSE PLATFORM_SOURCES "src/platform/*/windows/*.c")
else()
    file(GLOB_RECURSE PLATFORM_SOURCES "src/platform/*/linux/*.c")
endif()

set(PROBE_SOURCES
    ${LOADER_SOURCES}
    src/probe.c
    src/vulkan_smoke.c
    ${RUNTIME_SOURCES}
    ${RUNTIME_SUB_SOURCES}
    ${PLATFORM_SOURCES}
)

add_executable(bb-probe ${PROBE_SOURCES})

if (WIN32)
    target_compile_definitions(bb-probe PRIVATE _CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN NOMINMAX)
    target_link_libraries(bb-probe PRIVATE bbgpu atrac9 PkgConfig::SDL3 Vulkan::Vulkan)
else()
    target_compile_options(bb-probe PRIVATE -Wall -Wextra -Werror -no-pie -pthread)
    target_link_libraries(bb-probe PRIVATE bbgpu atrac9 PkgConfig::SDL3 PkgConfig::FFMPEG Vulkan::Vulkan m pthread)
endif()

# -----------------------------------------------------------------------------
# Utility: bb-gpu-capabilities
# -----------------------------------------------------------------------------
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tools/gpu_capabilities.c")
    add_executable(bb-gpu-capabilities
        tools/gpu_capabilities.c
        tools/gpu_capabilities/caps_device.c
        tools/gpu_capabilities/caps_formats.c)
    target_include_directories(bb-gpu-capabilities PRIVATE tools)
    if (WIN32)
        target_link_libraries(bb-gpu-capabilities PRIVATE Vulkan::Vulkan)
    else()
        target_compile_options(bb-gpu-capabilities PRIVATE -Wall -Wextra -Werror)
        target_link_libraries(bb-gpu-capabilities PRIVATE Vulkan::Vulkan m)
    endif()
endif()
