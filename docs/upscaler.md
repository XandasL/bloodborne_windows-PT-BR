# Temporal upscaling and frame generation — frame analysis and plan

## Native TAA and missing FSR 4 assets (2026-10-01)

`upscaler=taa` / `BB_UPSCALER=taa` is a separate native-resolution temporal AA
mode, available live in the launcher and overlay. It uses the existing jitter and
camera/object motion vectors, reprojection, depth rejection and 3×3 neighborhood
clipping in one compute pass with two ping-pong RGBA16F histories. History stores
depth in alpha; the scene's original alpha is preserved by the HDR merge. HUD is
composed afterwards and never enters history. Output changes and AA/provider
changes reset history. TAA uses Native AA dimensions while preserving the saved
FSR preset, and does not allocate an FSR context or execute an FSR model/RCAS.
It still adds GPU work compared with disabling temporal AA; native 4K raster cost
also remains. The explicit `BB_RENDER_RES` compatibility path must be unset.

FSR Native AA is not a fast pass-through: it renders at the output resolution and
also executes the temporal reconstruction (including the model for FSR 4) and
optional sharpening. It can therefore be slower than no AA at the same output.

FSR 4.1.1 assets have separate 1080p/2160p tiers and standard/Ultra models:
`t1080_m0`, `t1080_m1`, `t2160_m0`, `t2160_m1`. Outputs above 1080p need the 2160
tier, irrespective of the preset. A missing `t2160_m0/spd.spv` means missing
assets, not a GPU feature failure. These assets are not bundled: build the full
set with `tools/fsr4cap/build_assets.sh` and put it in `BB_FSR411_DIR` or the
packaged data directory's `fsr4_411`. Fatal asset failures now select FSR 3.1 in
the live settings, so the menu shows the active provider, retains the error, and
allows retrying FSR 4 after changing the output/installing assets.

`taa-shader-test` checks real SPIR-V output for accumulation, clipping, depth
disocclusion, invalid history and out-of-frame motion on Vulkan (Lavapipe works).
`out/taa-appimage-build.log` records these checks, settings round trips and 64
Python tests. `out/taa-live-validation.log` and `out/taa-appimage-validation.log`
check TAA at 1080p/720p/1440p/4K, TAA ↔ FSR 3, and a missing 2160-tier FSR 4.1.1
model in one gameplay process, including output captures and camera movement.
`out/taa-launcher-validation.log` checks actual GTK controls and saved settings.
TAA has not yet been tested on the tester's GTX 1060.

## Startup patch is the default again for outputs other than 1080p (2026-10-02)

The live path below made the Steam Deck and a GTX 1060 + 4-core Haswell drop to 7–8 FPS
(release 0.1 AppImage: 40–50 FPS on the Deck). With guest allocations at 1920×1080 the game's
post-processing stays at 1080p even for 720p output, guest compute passes resolve scene proxies
back to 1080p every frame (RX 7800 XT, 720p FSR 3 Performance on 4 cores: 826 vs 576
draws/frame, 185 vs 210 FPS), and on GPUs without shader stencil export every depth/stencil
copy took nine draws.

- `run.sh` again patches the game's render size for outputs other than 1080p (as in 0.1) and
  marks it with `BB_AUTO_RENDER_RES=1`, so an in-game restart recomputes it. Preset and output
  changes then need a restart (the menu offers it). `BB_LIVE_RES=1` selects the live path.
  1080p output and TAA keep the live path.
- Depth/stencil scene proxies need `VK_EXT_shader_stencil_export` again; without it these
  targets stay native (as in 0.1). `BB_SCENE_STENCIL_BITS=1` still forces the portable
  resampler for tests.

## Live resolution changes (2026-10-01)

The in-game output selector now resizes 720p/1080p/1440p/2160p host targets at a
display-pass boundary. Presets use `output / scale`, including raster sizes above
1080p (4K Quality: 2560×1440; Native AA: 3840×2160). Guest allocations stay at
1920×1080; `run.sh` no longer inserts a resolution patch for normal output choices.
The explicit `BB_RENDER_RES` compatibility override is retained.

`SceneTargets` resolves and retires old proxies before changing size. FSR resources
and history are rebuilt for the new input/output, with completion waits for both
FSR 3 and FSR 4. The 1080p path keeps HDR reconstruction before game post;
other outputs reconstruct tonemapped scene proxies at the first Scaleform pass,
then draw HUD/menu geometry into the output-size target. The display buffer is
resized too. Movie shader identification prevents a native-size fullscreen post
pass from being mistaken for UI.

Guest compute passes retain their native dispatch sizes and can resolve proxies
back to 1080p. This change does not scale every post-processing pass; performance
and intermediate detail can differ from the old startup-patched render sizes.

Verification: `out/dev/live-resolution-summary.log` switches output, preset and FSR
off/on in one PID. `BB_PRESENT_DUMP_TRIGGER` captures the completed host display
buffer (including HUD) into `BB_DUMP_DIR`. Captures confirm actual 720p/1440p/2160p
buffers. `scene-resolution-test` covers color/depth/stencil round trips through
proxies both smaller and larger than the guest allocation. `BB_PRESET_FILE` accepts
`preset [output-index [upscaler-index]]` for scripted menu-equivalent changes.

Frame analyzer: `BB_CAPTURE_TRIGGER=<file> BB_CAPTURE_DIR=<dir>`; creating the file records
the next frame (boundary: the pass writing a display buffer) — passes, targets, shaders,
sampled textures, and the first 1 KiB of bound constants for small passes and first draws.

## Bloodborne's frame (1920x1080, Hunter's Dream / Nightmare)

| Passes | What |
|---|---|
| first pass of a frame | copy of the previous UI target into the display buffer (sRGB) |
| shadow | 4096x4096 D32 depth |
| G-buffer | 6 targets 1920x1080 (RGBA8 x3, sRGB albedo, B10G11R11, RGBA16F) + D32S8 depth |
| lighting | light volumes into two B10G11R11 targets |
| scene color | RGBA16F 1920x1080, also forward/transparent draws and effects |
| volumetric fog | compute, reads linear depth (R32F 1920x1080) and composites into scene color |
| half-res effects | RGBA8 960x540 with half-res depth |
| post | combine/bloom pyramid in RGBA16F / B10G11R11 |
| tonemap | into RGBA8 1920x1080 (the UI target) |
| game AA | ping-pong RGBA8 1920x1080 after tonemap (to be skipped when upscaling) |
| UI | stencil-masked draws over the same RGBA8 target |

There is no velocity buffer, also with the camera moving: motion vectors are computed.

## Scene constants (864 bytes, bound by most passes)

Signature: `[0]=3000 (far) [1]=1/3000 [4]=1920 [5]=1080 [6]=1/1920 [7]=1/1080`.

| Floats | Meaning |
|---|---|
| 8–19 | view matrix, 3x4 rows (rotation + translation); the only block that changes with the camera |
| 36–51 | inverse projection (0.700285 = 1/1.42799, 0.39391 = 1/2.53865) |
| 52, 57, 62, 63 | projection (rows 52–67), D3D depth 0..1: x 1.42799, y 2.53865, z 1.00002 / -0.0500679 (near 0.05, far 3000) |
| 112–175 | shadow cascade matrices |
| 180–191 | inverse view (camera to world), 3x4 (176–179: 2, 8, 15, 0) |

The previous frame's matrices are not there; the port keeps them itself.

## Plan

1. Find the scene constants every frame (signature), keep the previous view/projection.
2. Camera motion vectors: compute pass from depth and current/previous matrices into an
   RG16F target; debug view to check them. Object motion later (vertex shader replay with the
   previous frame's constants through the recompiler).
3. Jitter: sub-pixel offset of clip-space positions in the scene passes (recompiler).
4. Upscaler at the scene color stage (before post and UI): FSR 3.1 first (open, native Vulkan,
   also frame generation), then DLSS (native on Linux), XeSS/XeFG and OptiScaler through a
   loader for their Windows DLLs.

## Status

- Camera motion vectors: verified by reprojecting the previous frame (`BB_DEBUG_MOTION=1`,
  toggles 1<<22 reprojected frame, 1<<23 error map): static geometry matches.
- FSR 3.1 (`BB_UPSCALER=fsr3`, FireBurn/FSR-Vulkan submodule): scene color before the
  post-processing combine (compute shader 9a9cf8a9), 1:1, RGB written back (the game keeps data
  in the alpha). Toggle 1<<24 switches it off at run time.
- Jitter: viewport offset of scene geometry (drawn with the scene depth, not full-screen
  quads), Halton(2,3) 8 phases; sign checked by sharpness (correct 255, flipped 208, off 271 —
  1:1 jitter trades high-frequency aliasing for a slightly softer image). Toggle 1<<25 off.
- Frame-to-frame difference while standing still: ~33% lower with FSR.
- Missing: motion of animated objects (characters, cloth, foliage), reactive/transparency
  masks for particles and fog, render-resolution scaling, frame generation.

## 2026-10-01: FSR 4 at 1440p/2160p Output Fix (Eliminating Shimmer & Jitter)

Recorded sample `video_2026-10-01_03-58-59.mp4` (3840x2160 output, FSR 4 Performance) exhibited severe shimmering and object shaking. Root cause: the upscaler pass was completely bypassed in this mode while viewport jitter remained active, sending raw stretched scene frames jittered across Halton phases directly to presentation.

1. The UI pass detection previously checked for an exact `BB_RENDER_RES` size (1916x1078), whereas the engine allocates render targets aligned to 8 pixels (1916x1080; scene constants also declare 1916x1080). `RunScaled` was never executed (no `UI: native composition` log entries). Alignment tolerance up to 8 pixels (`RenderTarget`) was added, and scene dimensions for FSR are now read directly from scene constants (`CameraMotion::RenderSize`, `SceneSize`).
2. FSR 4 subsequently failed with `external image registration failed (-1000069000)`: `RunScaled` was creating fresh image views every frame exceeding the internal 8-slot registry. Replaced with persistent `CachedView` instances matching the Native AA execution path.

Validation via frame dump (`BB_DUMP_TRIGGER=<file> BB_DUMP_DIR=<dir>`, `BB_DUMP_FRAMES=8` by default): with static camera, output PSNR between consecutive frames measures ~45 dB versus ~28 dB for jittered raw input. No ghosting trails observed during camera panning or movement. FPS in this mode is ~107 instead of ~220 (confirming active neural inference workload).

Remaining refinement: sprites (4-index draws) writing scene color with scene depth (glows, lantern lights) remain unjittered under the heuristic rule (`indices <= 6 = full-screen quad pass`).

## 2026-10-01: FSR 4 Post-Pass Optimization (2.9 ms → 0.8 ms in 4K)

Profiling measurements: `BB_FSR4_PROFILE=1` (pass breakdown timer via patched provider in `gpu/patches/fsr-vulkan`), standalone GPU benchmark `out/gpu/fsr4-bench` (`--stats` register and instruction report from RADV). Under 4K Balanced (2260x1272 → 3840x2160) on Radeon RX 7800 XT: total FSR 4 execution was 5.8 ms, with **post-pass taking 2.5–2.9 ms** (post-processing: final layers, 2x2 pixel shuffle, and history blending), 12 neural model passes taking 2.4 ms, and pre-pass 0.55 ms.

Bottleneck diagnosis: each thread computed a 2x2 block of output pixels and wrote them individually into three separate images (recurrent state, history, output), causing strided wave store instructions. Removing any one of the three write operations accelerated the pass by 1.3x to 4x.

Solution: `tools/fsr4_optimize.sh` decompiles the post-pass using SPIRV-Cross, `tools/fsr4_post_lds.pl` aggregates a 16x16 workgroup block in Local Data Share (LDS / shared memory) writing coalesced contiguous rows, and glslang recompiles it back into `fsr4_shaders/opt/`; `vk_fsr4.cpp` loads the optimized shader (`BB_FSR4_OPT=0` falls back to upstream original). Key implementation findings:
- SPIRV-Cross incorrectly translates signed int8 unpacking (`OpBitcast` to `i8vec4`) as unsigned `unpack8(uint)` — without correction, output degrades significantly (PSNR drops to 17 dB).
- Shared memory values must remain 32-bit float: 16-bit half types in LDS result in unorm8 rounding mismatches.

`tools/fsr4_verify.sh` verified bit-exact parity between original and optimized shaders for all presets across 1080p/1440p/4K: 1440p and 4K match bit-identically; post-pass latency reduced from 2.1–3.5 ms → 0.81–0.87 ms (4K), 1.2–1.5 ms → 0.36–0.38 ms (1440p), and 0.37–0.69 ms → 0.20–0.22 ms (1080p). In-game (4K Balanced), total FSR 4 frame time dropped from 5.8 ms to 4.0 ms, reducing total GPU frame time from 13.1 ms to ~11.6 ms.

Incidental discovery: **Upstream FSR 4 at 1080p output exhibits non-deterministic left-border artifacting** (columns 0–132) due to an uninitialized boundary read in the model shader.

Reactivity mask computation is skipped when running FSR 4, as the FSR 4 pipeline does not consume it.

### Additional Verification Notes (4K Balanced)

- **In-process A/B testing** (`ab.sh`, bit 24 — FSR disabled, direct UI copy without upscaling): 86.5 FPS with FSR 4 versus 87.7 FPS without. Post-pass optimization shifts the primary bottleneck to CPU dispatch rather than GPU compute. Async compute overlap for FSR is deferred pending further CPU runtime optimizations.
- **WMMA (`VK_KHR_cooperative_matrix`) exploration**: Prototype evaluation of Pass 1 (16-channel residual block: 3x3 16→16, 1x1 16→32 ReLU, 1x1 32→16) executed in 0.85 ms initial, 0.49 ms with LDS tile layout, compared to 0.30 ms for baseline DP4A/dot4. Tile layout translation overhead negated arithmetic throughput gains at 16-channel width.
- `RADV_PERFTEST=cswave32`: FSR 4 was slower (4.2 ms → 5.4 ms).
- Pre-pass (0.54 ms) is compute-bound, not write-bandwidth-limited (0.50 ms without store).

## 2026-10-01: FSR 4 Left-Border Flicker at 1080p Output — Pass 11 Workgroup Race

The 1080p non-determinism was traced to a race condition in the v07 decoder shader (Pass 11, 1/4 → 1/2 resolution). Each thread processes one 1/4-resolution input pixel and emits a 2x2 output block. Workgroups round up to 64 threads, but excess threads outside image width were not early-exited: at 1080p (480 input width), threads 480–511 wrote output pixels 960–1023 into the next scanline (15,360-byte tensor stride), racing against the valid scanline workers. At 4K output, 960 divides evenly by 64, avoiding the bug.

`tools/fsr4_pass11_guard.pl` inserts boundary guard conditions; `tools/fsr4_optimize.sh` compiles the guarded shader into `fsr4_shaders/opt` for all presets; verified via `tools/fsr4_verify.sh` to ensure bit-exact reproducibility.

## 2026-10-01: Native Vulkan FSR 4.1.1 Engine (`upscaler=fsr411`)

The AMD FSR 4.1.1 (INT8) neural model runs natively in Vulkan with bit-exact parity against reference runtime binaries.

**Architecture:** `tools/fsr4cap/fsr4cap.exe` captures the complete pipeline state under Vulkan/D3D12 translation: 29 dispatches per frame (SPD auto-exposure, prepass, pass0_post, model passes 1–12 with intermediate tensor clear passes, postpass, and RCAS).
- Dual models: m0 (Native through Performance) and m1 (Ultra Performance, 3.0x scale factor), with 128 KiB weights loaded via `InitializerBuffer`. Resolution tiers: t1080 (up to 1920x1080) and t2160 (1440p and 4K).
- Tensor dimensions align to 8-pixel boundaries; `CsTensorSizes` contains 17 dimension configurations validated across 20 capture dumps.
- dxil-spirv translates DXIL to Vulkan SPIR-V utilizing `--mixed-float-dot-product` (`VK_VALVE_shader_mixed_float_dot_product`, half dot2 with float accumulator), achieving identical output parity.
- `fsr411/fsr411.cpp` provides a standalone Vulkan execution core used by both the game runtime and `fsr4-bench --fsr411`.
- Post-pass LDS re-assembly (`postpass_lds.py`) reduces 4K pass execution time from 2.18 ms to 0.95 ms. Total FSR 4.1.1 latency: 1.09 ms at 1080p, 4.12 ms at 4K Balanced.

