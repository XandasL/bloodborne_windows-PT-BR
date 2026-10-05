# bbport GPU library: shadPS4 video core + shader recompiler + Gnm/VideoOut
# drivers, adapted to the native_probe runtime through shims in shim/.
cmake_minimum_required(VERSION 3.24)
project(bbgpu LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
if (MSVC)
    add_compile_options(/Oy-)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-fno-omit-frame-pointer)
endif()

# bbport: link-time optimization (BB_LTO, default on) and profile-guided optimization
# (BB_PGO: off, generate = instrumented build writing into BB_PGO_DIR, use = build with that
# profile). Covers bbgpu and the static libraries linked into it. No -march: the same build
# runs on the Steam Deck.
option(BB_LTO "Link-time optimization of bbgpu" ON)
set(BB_PGO "off" CACHE STRING "Profile-guided optimization: off, generate, use")
set(BB_PGO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../pgo" CACHE PATH "Profile directory")
if (BB_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT bb_ipo_ok OUTPUT bb_ipo_error)
    if (bb_ipo_ok)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    else()
        message(WARNING "LTO unavailable: ${bb_ipo_error}")
    endif()
endif()
if (BB_PGO STREQUAL "generate")
    # Atomic counters: the GPU, recording, preparation and copy threads run the same code.
    add_compile_options(-fprofile-generate=${BB_PGO_DIR} -fprofile-update=atomic)
    add_link_options(-fprofile-generate=${BB_PGO_DIR})
    add_compile_definitions(BB_PGO_GENERATE)
elseif (BB_PGO STREQUAL "use")
    # Partial training keeps unprofiled code optimized normally; counters from concurrent
    # threads can be slightly inconsistent (correction). Functions changed since the profile
    # (or built differently for it, bbgpu_init) are compiled without it.
    add_compile_options(-fprofile-use=${BB_PGO_DIR} -fprofile-partial-training
                        -fprofile-correction -Wno-missing-profile -Wno-coverage-mismatch)
endif()

if (POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()
if (POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()

include(FetchContent)

find_package(Vulkan REQUIRED)
FetchContent_Declare(
    VulkanHeaders
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
    GIT_TAG v1.3.296
)
FetchContent_MakeAvailable(VulkanHeaders)

# Boost headers discovery and fallback
find_package(Boost QUIET)
if (NOT TARGET Boost::headers)
    if (DEFINED ENV{BOOST_ROOT} AND EXISTS "$ENV{BOOST_ROOT}")
        set(BOOST_ROOT "$ENV{BOOST_ROOT}")
        set(BOOST_INCLUDEDIR "$ENV{BOOST_ROOT}")
        find_package(Boost QUIET)
    endif()
endif()
if (NOT TARGET Boost::headers)
    file(GLOB BOOST_CANDIDATES "C:/boost*" "C:/local/boost*" "$ENV{BOOST_ROOT}")
    foreach(CANDIDATE ${BOOST_CANDIDATES})
        if (EXISTS "${CANDIDATE}/boost/version.hpp")
            add_library(Boost::headers INTERFACE IMPORTED GLOBAL)
            target_include_directories(Boost::headers INTERFACE "${CANDIDATE}")
            break()
        elseif (EXISTS "${CANDIDATE}/include/boost/version.hpp")
            add_library(Boost::headers INTERFACE IMPORTED GLOBAL)
            target_include_directories(Boost::headers INTERFACE "${CANDIDATE}/include")
            break()
        endif()
    endforeach()
endif()
if (NOT TARGET Boost::headers)
    message(STATUS "Boost headers not found on system. Fetching header-only Boost 1.84.0...")
    FetchContent_Declare(
        boost_headers
        URL https://github.com/boostorg/boost/releases/download/boost-1.84.0/boost-1.84.0.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(boost_headers)
    add_library(Boost::headers INTERFACE IMPORTED GLOBAL)
    target_include_directories(Boost::headers INTERFACE ${boost_headers_SOURCE_DIR})
endif()

# 1. fmt
find_package(fmt 10.0 QUIET)
if (NOT TARGET fmt::fmt)
    FetchContent_Declare(fmt GIT_REPOSITORY https://github.com/fmtlib/fmt.git GIT_TAG 10.2.1)
    FetchContent_MakeAvailable(fmt)
endif()

# 2. magic_enum
find_package(magic_enum QUIET)
if (NOT TARGET magic_enum::magic_enum)
    FetchContent_Declare(magic_enum GIT_REPOSITORY https://github.com/Neargye/magic_enum.git GIT_TAG v0.9.5)
    FetchContent_MakeAvailable(magic_enum)
    if (TARGET magic_enum AND NOT TARGET magic_enum::magic_enum)
        add_library(magic_enum::magic_enum ALIAS magic_enum)
    endif()
    if (TARGET magic_enum)
        target_include_directories(magic_enum INTERFACE ${magic_enum_SOURCE_DIR}/include)
    endif()
endif()

# 3. tsl-robin-map
find_package(tsl-robin-map QUIET)
if (NOT TARGET tsl::robin_map)
    FetchContent_Declare(tsl_robin_map GIT_REPOSITORY https://github.com/Tessil/robin-map.git GIT_TAG v1.3.0)
    FetchContent_MakeAvailable(tsl_robin_map)
    if (TARGET tsl_robin_map AND NOT TARGET tsl::robin_map)
        add_library(tsl::robin_map ALIAS tsl_robin_map)
    endif()
endif()

# 4. VulkanMemoryAllocator
find_package(VulkanMemoryAllocator CONFIG QUIET)
if (NOT TARGET GPUOpen::VulkanMemoryAllocator)
    FetchContent_Declare(VulkanMemoryAllocator GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git GIT_TAG v3.0.1)
    FetchContent_MakeAvailable(VulkanMemoryAllocator)
    if (TARGET VulkanMemoryAllocator AND NOT TARGET GPUOpen::VulkanMemoryAllocator)
        add_library(GPUOpen::VulkanMemoryAllocator ALIAS VulkanMemoryAllocator)
    endif()
endif()

if (NOT WIN32)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(XXHASH REQUIRED IMPORTED_TARGET libxxhash)
    pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3)
    pkg_check_modules(X11 REQUIRED IMPORTED_TARGET x11)
    pkg_check_modules(FFMPEG IMPORTED_TARGET libavcodec libavutil)
    if (TARGET PkgConfig::SDL3)
        set_target_properties(PkgConfig::SDL3 PROPERTIES IMPORTED_GLOBAL TRUE)
    endif()
else()
    find_package(PkgConfig QUIET)
    if (PkgConfig_FOUND)
        pkg_check_modules(XXHASH IMPORTED_TARGET libxxhash)
        pkg_check_modules(SDL3 IMPORTED_TARGET sdl3)
    endif()
    if (NOT TARGET PkgConfig::SDL3)
        find_package(SDL3 CONFIG QUIET)
        if (TARGET SDL3::SDL3)
            add_library(PkgConfig::SDL3 ALIAS SDL3::SDL3)
        else()
            set(SDL_TESTS OFF CACHE BOOL "" FORCE)
            set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
            FetchContent_Declare(SDL3 GIT_REPOSITORY https://github.com/libsdl-org/SDL.git GIT_TAG release-3.2.8)
            FetchContent_MakeAvailable(SDL3)
            if (TARGET SDL3-shared)
                add_library(PkgConfig::SDL3 ALIAS SDL3-shared)
            elseif (TARGET SDL3)
                add_library(PkgConfig::SDL3 ALIAS SDL3)
            endif()
        endif()
    endif()
    if (NOT TARGET PkgConfig::XXHASH)
        find_package(xxHash CONFIG QUIET)
        if (TARGET xxHash::xxhash)
            add_library(PkgConfig::XXHASH ALIAS xxHash::xxhash)
        else()
            FetchContent_Declare(xxHash GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git GIT_TAG v0.8.2)
            FetchContent_MakeAvailable(xxHash)
            if (TARGET xxHash::xxhash)
                add_library(PkgConfig::XXHASH ALIAS xxHash::xxhash)
            elseif (TARGET xxHash)
                add_library(PkgConfig::XXHASH ALIAS xxHash)
            elseif (TARGET xxhash)
                add_library(PkgConfig::XXHASH ALIAS xxhash)
            else()
                add_library(xxhash_static STATIC ${xxhash_SOURCE_DIR}/xxhash.c)
                target_include_directories(xxhash_static PUBLIC ${xxhash_SOURCE_DIR})
                add_library(PkgConfig::XXHASH ALIAS xxhash_static)
            endif()
        endif()
    endif()
endif()

# 5. miniz
find_package(miniz QUIET)
if (NOT TARGET miniz::miniz)
    FetchContent_Declare(miniz GIT_REPOSITORY https://github.com/richgel999/miniz.git GIT_TAG 3.0.2)
    FetchContent_MakeAvailable(miniz)
    if (TARGET miniz AND NOT TARGET miniz::miniz)
        add_library(miniz::miniz ALIAS miniz)
    endif()
endif()

# 6. Zydis
find_package(Zydis QUIET)
if (NOT TARGET Zydis::Zydis)
    set(ZYDIS_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    set(ZYDIS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(Zydis GIT_REPOSITORY https://github.com/zyantific/zydis.git GIT_TAG v4.1.0)
    FetchContent_MakeAvailable(Zydis)
    if (TARGET Zydis AND NOT TARGET Zydis::Zydis)
        add_library(Zydis::Zydis ALIAS Zydis)
    endif()
endif()

find_program(GLSLANG_COMPILER NAMES glslang glslangValidator glslc REQUIRED)

set(SIRIT_USE_SYSTEM_SPIRV_HEADERS OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/sirit EXCLUDE_FROM_ALL)

# bbport: FSR 3.1 temporal upscaling / frame generation, native Vulkan (MIT, FireBurn/FSR-Vulkan).
set(FFX_VK_PORTABLE_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FFX_VK_PORTABLE_BUILD_PROBE OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/fsr-vulkan EXCLUDE_FROM_ALL)

# Host shaders -> headers (same scheme as shadPS4's host_shaders/CMakeLists.txt).
set(SHADER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/shadps4/video_core/host_shaders)
set(SHADER_INCLUDE ${CMAKE_CURRENT_BINARY_DIR}/include)
set(SHADER_DIR ${SHADER_INCLUDE}/video_core/host_shaders)
set(SHADER_HEADERS "")
function(add_host_shader FILENAME SHADER_NAME)
    set(OUT ${SHADER_DIR}/${SHADER_NAME}.h)
    string(TOUPPER ${SHADER_NAME} SYMBOL)
    set(DEFINES ${ARGN})
    list(TRANSFORM DEFINES PREPEND "-D")
    add_custom_command(OUTPUT ${OUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADER_DIR}
        COMMAND ${GLSLANG_COMPILER} --target-env vulkan1.3 -o ${OUT} --vn ${SYMBOL} ${DEFINES} ${SHADER_SRC}/${FILENAME}
        COMMAND ${CMAKE_COMMAND} "-DHEADER_FILE=${OUT}" -P ${SHADER_SRC}/AddShaderHeaderInclude.cmake
        DEPENDS ${SHADER_SRC}/${FILENAME} VERBATIM)
    set(SHADER_HEADERS ${SHADER_HEADERS} ${OUT} PARENT_SCOPE)
endfunction()
add_host_shader(color_to_ms_depth.frag color_to_ms_depth_frag)
add_host_shader(ms_image_blit.frag ms_image_blit_frag)
add_host_shader(ms_image_blit.frag ms_image_blit_msaa_frag SRC_MSAA=1)
add_host_shader(fault_buffer_process.comp fault_buffer_process_comp)
add_host_shader(fs_tri.vert fs_tri_vert)
add_host_shader(fsr.comp fsr_easu_comp SAMPLE_EASU=1)
add_host_shader(fsr.comp fsr_rcas_comp SAMPLE_RCAS=1)
add_host_shader(post_process.frag post_process_frag)
add_host_shader(camera_motion.comp camera_motion_comp)
add_host_shader(upscale_merge.comp upscale_merge_comp)
add_host_shader(upscale_reactive.comp upscale_reactive_comp)
add_host_shader(taa.comp taa_comp)
add_host_shader(taa_sharpen.comp taa_sharpen_comp)
add_host_shader(taa_sharpen.comp taa_sharpen_ldr_comp LDR_OUTPUT=1)
add_host_shader(buffer_multi_copy.comp buffer_multi_copy_comp)
add_host_shader(depth_resample.frag depth_resample_frag)
add_host_shader(depth_resample.frag depth_stencil_resample_frag HAS_STENCIL=1)
add_host_shader(depth_resample.frag depth_stencil_bits_frag STENCIL_BITS=1)
add_host_shader(camera_motion.comp camera_motion_debug_comp DEBUG_OVERLAY=1)
foreach(BPP IN ITEMS 8 16 32 64 96 128)
    add_host_shader(tiling.comp tiling_micro_${BPP}_comp BITS_PER_PIXEL=${BPP} IS_MACRO_TILED=0)
    add_host_shader(tiling.comp tiling_macro_${BPP}_comp BITS_PER_PIXEL=${BPP} IS_MACRO_TILED=1)
endforeach()
add_custom_target(host_shaders DEPENDS ${SHADER_HEADERS})

file(GLOB_RECURSE VIDEO_CORE CONFIGURE_DEPENDS shadps4/video_core/*.cpp)
file(GLOB_RECURSE RECOMPILER CONFIGURE_DEPENDS shadps4/shader_recompiler/*.cpp)
list(FILTER VIDEO_CORE EXCLUDE REGEX "renderdoc\\.cpp$")
set(COMMON
    shadps4/common/decoder.cpp shadps4/common/error.cpp
    shadps4/common/io_file.cpp shadps4/common/native_clock.cpp shadps4/common/number_utils.cpp
    shadps4/common/path_util.cpp shadps4/common/rdtsc.cpp shadps4/common/spin_lock.cpp shadps4/common/recursive_lock.cpp
    shadps4/common/string_util.cpp shadps4/common/thread.cpp shadps4/common/signal_context.cpp
    shim/common/logging/log.cpp)
file(GLOB AVPLAYER CONFIGURE_DEPENDS shadps4/core/libraries/avplayer/*.cpp)
if (NOT WIN32)
    pkg_check_modules(FFMPEG REQUIRED IMPORTED_TARGET libavformat libavcodec libavutil libswscale libswresample)
endif()
file(GLOB SHIM_SOURCES CONFIGURE_DEPENDS shim/*.cpp shim/*/*.cpp)
add_library(bbgpu SHARED ${VIDEO_CORE} ${RECOMPILER} ${COMMON} ${SHIM_SOURCES}
    shadps4/core/libraries/gnmdriver/gnmdriver.cpp
    shadps4/core/libraries/videoout/driver.cpp shadps4/core/libraries/videoout/video_out.cpp
    shadps4/core/libraries/kernel/equeue.cpp ${AVPLAYER}
    shadps4/core/libraries/videodec/video_utils.cpp)
add_dependencies(bbgpu host_shaders)
# In-game settings menu: Dear ImGui (third_party/imgui, MIT) with its Vulkan backend, and an
# embedded Cyrillic font (third_party/fonts).
target_sources(bbgpu PRIVATE third_party/imgui/imgui.cpp third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_vulkan.cpp)
target_include_directories(bbgpu PRIVATE third_party/imgui third_party/imgui/backends)
target_compile_definitions(bbgpu PRIVATE IMGUI_IMPL_VULKAN_NO_PROTOTYPES
    BB_FONT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/third_party/fonts/DejaVuSans.ttf")
set_source_files_properties(shim/bbport_overlay.cpp PROPERTIES
    OBJECT_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/fonts/DejaVuSans.ttf)
target_include_directories(bbgpu PRIVATE shim shadps4 ${SHADER_INCLUDE} third_party/gcn/include third_party/half/include ${magic_enum_SOURCE_DIR}/include)
target_compile_definitions(bbgpu PRIVATE BOOST_ASIO_STANDALONE NDEBUG)
if (MSVC)
    target_compile_options(bbgpu PRIVATE /W0)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bbgpu PRIVATE -Wno-deprecated-declarations -Wno-missing-field-initializers)
    if (NOT WIN32)
        target_link_options(bbgpu PRIVATE -Wl,--allow-shlib-undefined)
    endif()
endif()
target_link_libraries(bbgpu PRIVATE ffx-vulkan::portable ffx-vulkan::fsr4-v07-vulkan)
target_link_libraries(bbgpu PUBLIC Vulkan::Headers fmt::fmt Boost::headers magic_enum::magic_enum
    tsl::robin_map GPUOpen::VulkanMemoryAllocator PkgConfig::XXHASH PkgConfig::SDL3 miniz::miniz Zydis::Zydis sirit)
if (UNIX AND NOT APPLE)
    target_link_libraries(bbgpu PUBLIC PkgConfig::X11 PkgConfig::FFMPEG)
endif()
if (WIN32)
    target_compile_definitions(bbgpu PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN VK_USE_PLATFORM_WIN32_KHR)
endif()

# Explicit test targets; neither launches the game nor needs a Vulkan device.
add_executable(motion-history-test EXCLUDE_FROM_ALL ../tests/test_motion_history.cpp)
target_compile_options(motion-history-test PRIVATE -UNDEBUG -Wall -Wextra -Werror)
target_include_directories(motion-history-test PRIVATE ..)
add_executable(motion-shader-test EXCLUDE_FROM_ALL ../tests/test_motion_shaders.cpp ../tests/test_gpu_runtime_stubs.cpp)
target_include_directories(motion-shader-test PRIVATE shim shadps4 third_party/gcn/include)
target_link_libraries(motion-shader-test PRIVATE bbgpu)
add_executable(ui-composition-test EXCLUDE_FROM_ALL ../tests/test_ui_composition.cpp)
target_compile_options(ui-composition-test PRIVATE -UNDEBUG -Wall -Wextra -Werror)
target_include_directories(ui-composition-test PRIVATE ..)

add_executable(scene-resolution-test EXCLUDE_FROM_ALL ../tests/test_scene_resolution.cpp ../tests/test_gpu_runtime_stubs.cpp)
target_include_directories(scene-resolution-test PRIVATE shim shadps4 third_party/gcn/include)
# The test's Vulkan-Hpp inline methods have a different set of platform extensions
# from vk_platform.cpp. Keep them local and use an explicit dispatcher in the test.
target_compile_options(scene-resolution-test PRIVATE -UNDEBUG -fPIC)
set_source_files_properties(../tests/test_scene_resolution.cpp PROPERTIES
    COMPILE_OPTIONS "-fvisibility=hidden;-fvisibility-inlines-hidden")
target_link_libraries(scene-resolution-test PRIVATE bbgpu)
add_executable(taa-shader-test EXCLUDE_FROM_ALL ../tests/test_taa_shader.cpp ../tests/test_gpu_runtime_stubs.cpp)
target_include_directories(taa-shader-test PRIVATE shim shadps4 ${SHADER_INCLUDE} third_party/gcn/include)
target_compile_options(taa-shader-test PRIVATE -UNDEBUG -fPIC)
set_source_files_properties(../tests/test_taa_shader.cpp PROPERTIES
    COMPILE_OPTIONS "-fvisibility=hidden;-fvisibility-inlines-hidden")
target_link_libraries(taa-shader-test PRIVATE bbgpu)
add_dependencies(taa-shader-test host_shaders)

add_executable(camera-motion-test EXCLUDE_FROM_ALL ../tests/test_camera_motion.cpp ../tests/test_gpu_runtime_stubs.cpp)
target_include_directories(camera-motion-test PRIVATE shim shadps4 ${SHADER_INCLUDE} third_party/gcn/include)
target_compile_options(camera-motion-test PRIVATE -UNDEBUG -fPIC)
target_link_libraries(camera-motion-test PRIVATE bbgpu)
add_dependencies(camera-motion-test host_shaders)

# bbport: FSR 4 benchmark outside the game (tools/fsr4_bench.cpp): ninja -C out/gpu fsr4-bench.
add_executable(upscaler-support-test EXCLUDE_FROM_ALL ../tests/test_upscaler_support.cpp
    shim/bbport_settings.cpp)
target_include_directories(upscaler-support-test PRIVATE shim)
target_compile_options(upscaler-support-test PRIVATE -UNDEBUG)

add_executable(fsr4-bench EXCLUDE_FROM_ALL tools/fsr4_bench.cpp
    shadps4/video_core/renderer_vulkan/fsr411/fsr411.cpp)
target_include_directories(fsr4-bench PRIVATE shadps4/video_core/renderer_vulkan/fsr411)
target_link_libraries(fsr4-bench PRIVATE ffx-vulkan::fsr4-v07-vulkan ffx-vulkan::fsr4-v07-assets Vulkan::Vulkan)
