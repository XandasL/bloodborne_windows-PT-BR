# Bloodborne on PC: Architecture and Upscaler Status Audit

Snapshot as of **September 29, 2026**.

## Executive Summary

The project is an experimental **native Linux runtime**, not an emulator: original x86-64 `CUSA03173/eboot.bin` executes natively on the host CPU, PS4 system calls are handled by a modular C HLE runtime, and graphics are rendered through a custom Vulkan core based on shadPS4.

**FSR 3.1 temporal upscaling operates via the established launch-time scene resolution patch**: logs demonstrate `960×540 → 1920×1080` scaling, native 1080p UI composition, and active testing on Radeon RX 7800 XT.

## Architecture Breakdown

| Path | Component Role |
|---|---|
| `src/loader/`, `src/runtime/`, `src/probe.c` | Executable image preparation, module rebasing, ELF loader, and OS HLE syscalls. |
| `gpu/` | Vulkan video core, shader recompiler, Bloodborne optimizations, FSR, and Dear ImGui. |
| `patches/Bloodborne.xml`, `patches.py` | Framerate and resolution patches applied to guest code at startup. |
| `bbport.ini`, `run.sh`, `CMakeLists.txt` | Configuration, launch scripts, and root CMake configuration. |

## Subsystem Details

### Execution and Performance
- Original `eboot.bin` and select PRX modules run alongside custom implementations of memory management, thread scheduling, filesystem IO, synchronization primitives, gamepads, audio, and save state persistence. Network functions in offline mode.
- SDL3 handles gamepad input and fallback keyboard mapping. Audio is output through host ALSA/PulseAudio/WASAPI.
- Vulkan renderer features a dedicated command recording thread, 4-worker draw preparation, texture and buffer caching, asynchronous guest memory copy workers, and a pooled staging buffer allocator.

### Temporal Upscaling & Motion Vectors
- Camera motion vectors are derived from depth buffer deltas and inverse view-projection transforms. Geometry viewport jitter uses Halton(2,3) subpixel sequences.
- FSR 3.1 integrated via `gpu/third_party/fsr-vulkan`. Native AA executes 1:1 on HDR scene color prior to post-processing.
- Presets: Native AA `1920×1080`, Quality `1280×720`, Balanced `1130×636`, Performance `960×540`, Ultra Performance `640×360`.

## September 29 Refinement: Dynamic Preset Switching
- Replaced missing hardware `D32S8` blit with full-screen fragment pass (`depth_resample.frag`) using `gl_FragDepth` and `VK_EXT_shader_stencil_export`.
- Corrected reactivity mask to sample downscaled scene targets rather than full-res buffers.

## September 30 Milestone: FSR 4 INT8 & Command Dispatch Profiling
- Integrated FSR 4 (v07 INT8/DOT4) neural upscaler via `vk_fsr4.cpp/.h` using `shaderIntegerDotProduct` and `VK_KHR_compute_shader_derivatives`.
- CPU command thread profiling: format properties caching in `SceneTargets` and scheduling through `Scheduler::Record` reduced GPU command thread utilization from ~94% to ~74% of a core, improving framerates from 53–87 FPS to 61–98 FPS.
- Added `Motion::IndexRangeCache`: cached vertex index bounding boxes reduced index scanning overhead from ~10% to negligible.
