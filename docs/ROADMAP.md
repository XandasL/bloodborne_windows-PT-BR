# Native Bloodborne Port: Progress and Roadmap

> **Fork and Attributions:** This repository is a fork of [deadinside28/bloodborne_pc](https://github.com/deadinside28/bloodborne_pc) featuring deep architectural refactoring for native execution on Windows and a modular codebase structure ($\le 75$ lines per file).

Main text reflects architecture state; below is the summary of architectural milestones and next steps.
Most detailed milestone audit: [CURRENT_STATUS_2026-09-29.md](history/CURRENT_STATUS_2026-09-29.md).
Session report (GPU pipeline, upscaler, stability, launcher, AppImage): [SESSION_2026-09-30.md](history/SESSION_2026-09-30.md).
Topic-specific deep dives:
[parallel_gpu.md](parallel_gpu.md) (multi-threaded GPU execution, streaming hitches),
[upscaler.md](upscaler.md) (game frame structure, scene constants, upscaling passes).

## Architecture & Upscaler Milestones

- **FSR 4 INT8 (v07) and DLSS integration**: Object motion vectors, live preset switching (downscaled scene targets; D32S8 depth copied via shader), LTO and PGO build integration, multi-threaded draw preparation scaled to core count.
- **Motion vectors**: Character and clothing ghosting eliminated across temporal upscalers (FSR 3/4, DLSS).
- **UI Resolution**: Full native UI rasterization (1080p / 1440p / 2160p) independent of internal scene rendering resolution.
- **Two-Stage Draw Pipeline** (`vk_draw_pipe.h`): GPU command thread decodes PM4 packets and selects pipelines; dedicated recording thread (`bb:DrawRec`) with its own register state copy binds resources and records draws. Yields +18% to +20% FPS scaling.
- **Preset & GPU Load Scaling**: Direct sampling of downscaled render targets without redundant re-sampling passes to native.
- **Stability & Heap Fixes**: Resolved EOP marker delays before GPU idle interrupts, preventing guest heap memory corruption.
- **Display Resolution Scaling (1440p / 2160p)**: Configurable output resolution via `bbport.ini` / launcher. Scene renders at preset fraction, temporal upscaler resolves to native target, and HUD/UI renders cleanly at output resolution. Game effects (aberration, DoF, blur, SSAO, dynamic shadows, SSR) are fully modular.
- **Launcher & Packaging**: Modern cross-platform launcher (GTK4/libadwaita on Linux, Win32 launcher support) and standalone deployment bundles.

## Port Architecture Overview

- Original `eboot.bin` (CUSA03173 v01.09) executes directly on x86-64. PS4 system libraries are replaced by a modular C runtime (`src/runtime/*.c`, `src/probe.c`).
- Graphics: Custom Vulkan video core and shader recompiler derived from shadPS4 (GPL), compiled into `libbbgpu.so` / `bbgpu.dll` (`gpu/`).
- Game patches (framerate unlocks, resolution patches, debug camera, etc.): Community XML patches (`patches/Bloodborne.xml`) applied by the runtime loader during startup.

## Implemented Optimizations

### Performance and Smoothness

| Feature | Impact |
|---|---|
| Vulkan command recording on dedicated thread, texture/buffer caches, read-write locks | 26 → 64 FPS in high-complexity scenes |
| Draw preparation across 4 worker threads (`bb:DrawPrep`) with GPU thread validation | +11.5% throughput |
| 256 KiB write-protect unguard window on game memory writes | +22% reduction in page fault overhead |
| Texture descriptor caching and fast-path for persistent render targets | Up to ~94 FPS in open-world areas |
| Thread-pooled guest memory copies and staging buffer pooling | Streaming hitch duration reduced from 60–170 ms to 40–50 ms |
| Copy synchronizations before game-visible fences | UI flickering eliminated |
| Relaxed Readbacks mode | Resolved vertex explosion artifacts (FaceGen) |
| 480 Hz Vblank timer with frame limiter to min(refresh_rate, 120) | Smooth frame pacing; movement speed normalized above 60 FPS |

All features can be toggled live via `BB_TOGGLE_FILE` bitmasks for A/B profiling. Metrics enabled via `BB_FRAME_STATS=1`.

### Audio and Input

- Audio playback pacing synchronized to frame clock (eliminating periodic audio stutter).
- Controller support via SDL3 with fallback keyboard bindings.

### Frame Analysis & Upscalers

- Single-frame capture triggers (`BB_CAPTURE_TRIGGER=<file>`) dumping passes, targets, shaders, textures, and scene constants.
- Full reverse-engineering of Bloodborne rendering passes (G-buffer, lighting, RGBA16F HDR scene color, volumetric fog, post-processing, tonemapping, game AA, HUD) and 864-byte scene constants (view and projection matrices).
- Camera motion vectors derived from depth buffer and inverted view-projection deltas (`camera_motion.comp`).
- Halton(2,3) subpixel jitter sequence applied to scene geometry viewport.
- **FSR 3.1 & FSR 4**: Full native Vulkan integration with Quality, Balanced, Performance, and Ultra Performance presets.
- **DLSS Bridge & NGX**: NVIDIA DLSS SDK integrated via standalone MIT DLL bridge (`dlss_bridge/`) loaded dynamically at runtime on supported GeForce RTX GPUs.
- **In-Game Overlay**: ImGui in-game menu accessible via Insert or L3+R3.
