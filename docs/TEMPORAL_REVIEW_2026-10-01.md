# Temporal Anti-Aliasing & Upscaler Verification (2026-10-01)

## Architecture Context

`bbport` is a specialized runtime executing native x86-64 code for Bloodborne (CUSA03173 1.09) translating GPU commands directly into Vulkan. System call HLE, audio, input, save state persistence, in-game ImGui overlay, and temporal upscaling modules are fully functional.

## Artifact Analysis & Root Cause Diagnosis

Test recording: `video_2026-10-01_21-21-02.mp4` (2560×1080 @ 60 FPS) cycling across TAA, FSR 4.1.1, FSR 4, and FSR 3.1. Architectural details and thin geometry demonstrated localized temporal instability.

Identified algorithmic bugs and fixes:

1. **FP16 Depth in TAA History.** The quantization step of 16-bit half precision near 1.0 is ~0.000488. With near clip ~0.05, distant surfaces lost precision. Disocclusion thresholds were smaller than the quantization step, erroneously discarding valid history or blending disparate surfaces. TAA history format upgraded to RGBA32F.
2. **Incompatible Camera Frames.** Prior tests evaluated previous frame depth directly against current frame depth. The current point is now explicitly transformed into the previous camera frame, comparing against expected reprojected depth with perspective-scaled tolerance.
3. **Distant Vector Zeroing.** `depth >= 0.99999` was erroneously treated as sky background, zeroing motion vectors. For projections with near=0.05, far=3000, this affected geometry beyond ~1875 units. Now, only clear depth 1.0 is treated as infinite background; background surfaces receive pure camera rotation vectors without translation.
4. **FP16 Depth in Object Motion.** In RGBA16F, depth precision was insufficient to determine if an object motion vector was occluded by a subsequent draw. Buffer and graphics pipeline formats upgraded to RGBA32F. Final FSR motion outputs remain RG16F.
5. **Mixed Depth Boundary Validation.** TAA interpolated history depth across surface silhouettes. Now, four bilinear taps are tested independently, accumulating color strictly from valid taps.
6. **Conservative Far-Distance Weighting.** Replaced previous aggressive history decay with a calibrated 0.98–0.85 weighting curve based on velocity. History RGB values are clamped to the neighborhood bounding box of the current frame.
7. **Depth Jitter on Slanted Surfaces.** Depth is aligned to stable pixel coordinates via local one-sided gradients. For motion/depth sampling, the nearest 3×3 surface is selected, preserving fine silhouettes during Halton subpixel jitter.
8. **Subpixel History Addressing Instability (Primary TAA Shimmer Cause).** Bilinear whole and fractional components were previously computed after summing integer pixel indices with subpixel velocity deltas, causing floating point precision discrepancies across wave threads. Motion offsets are now split prior to offset addition: `base = p + ivec2(floor(motion))`, `fraction = motion - floor(motion)`.

## Validation

- 18 test cases in the production TAA shader verified on Vulkan GPU: reset on NaN, unjitter, accumulation, neighborhood clipping, disocclusion, frustum bounds, non-zero distant depth, camera translation, partial coverage, sky/geometry boundaries, and subpixel motion addressing.
- 6 camera motion shader tests verified: distant geometry reprojection, rotational sky vectors, jitter cancellation, and valid/stale object vectors.
- Regression tests runnable via:
```sh
ninja -C out/gpu taa-shader-test camera-motion-test
out/gpu/taa-shader-test
out/gpu/camera-motion-test
```
