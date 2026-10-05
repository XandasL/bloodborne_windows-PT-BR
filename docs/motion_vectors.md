# Motion Vectors: Post-Video Refinement (2026-09-26)

Test source: `video_2026-09-26_04-19-53.mp4`, FSR 3.1 Performance, 960×540 internal render → 1920×1080 display, and `out/dev/t71.log`.

Global object motion vector passes across all G-buffer draws were excessively expensive: Performance preset delivered ~49 FPS with global motion passes versus 77–78 FPS without. Object motion vectors are now selectively enabled by default only for shaders referencing bone/pose buffers (size 640–16384 bytes, excluding the 864-byte global scene buffer; 16-byte alignment). With a tuned threshold (1024 bytes), processed draws per frame dropped to ~140, eliminating history capacity overflow and achieving ~81–87 FPS. Remaining draws utilize camera motion vectors. `BB_OBJECT_MOTION=0` disables the feature completely; `BB_OBJECT_MOTION_ALL=1` restores the global pass for A/B profiling. Alpha channel in the velocity target stores scene depth: if a later static draw occludes an object, its motion vector is rejected.

## Initial Observations

- Over 600 sample frames: 257,400 total G-buffer draws; 256,200 flagged moving, but only 30,000 received previous positions (~11.7% draw coverage).
- Memory reservation allocated entire vertex buffer sizes multiplied by instance counts inclusive of `firstInstance`, wasting budget for sub-meshes sharing buffers (capped at 2,097,152 positions/frame).
- Missing vertex-to-vertex synchronization barrier between shader writes/reads; host parameter ring buffer reuse did not wait on GPU fence completion.
- Viewport jitter offset was omitted when reconstructing clip positions from depth buffers (`CameraMotion::OnDisplayPass`).

## Architectural Changes

- History tracking ranges are bounded by min/max index values accounting for `baseVertex` and primitive restart. `FirstInstance` offset is subtracted in the shader rather than inflating buffer allocations.
- Heuristic constant-delta detection was replaced with direct topological matching: all eligible direct G-buffer draws are tracked from first appearance. History lookup validates match against previous frame.
- Lookup key incorporates all vertex stream descriptors, shader hash, index address and content hash, vertex range, instance count, and draw sequence index. Topology shifts or dropped frames flush matching history entries.
- Position budget expanded to 4,194,304 positions/frame (128 MiB across both ping-pong buffers). Exhaustion falls back cleanly to camera motion vectors.
- Added pipeline vertex-to-vertex barriers, GPU fence synchronization prior to host parameter reuse, and coherent host memory layout tracking.
- Branching early-outs bypass unmapped invocations; atomic coordinate component writes handle repeated indexed vertex invocations. Behind-camera coordinates and NaN/Inf vectors are filtered out.
- Viewport jitter phase sequence configured to 32 phases under Performance scaling. Camera motion vectors are reconstructed using unjittered clip coordinates.

## Verification

Build and unit tests:

```bash
ninja -C out/gpu motion-history-test motion-shader-test
out/gpu/motion-history-test
out/gpu/motion-shader-test out/motion-shaders
```

`test_motion_history.cpp` tests index ranges (u16/u32), primitive restart, negative baseVertex, firstInstance, duplicate draws, topology switches, dropped frames, capacity limits, and phase count under ASan/UBSan.

`test_motion_shaders.cpp` emits VS/PS pairs with and without motion vectors through the SPIR-V backend, validating output via `spirv-val --target-env vulkan1.3`.

## In-Game Execution

```bash
BB_FRAME_STATS=1 BB_TOGGLE_FILE=out/dev/motion-toggles ./bb-probe
```

Bit 29 in the toggle file disables history recording at runtime. For profiling, compare runs with and without `BB_OBJECT_MOTION=0`.

## Whirligig Saw & Small Skeletons (Ghosting Resolution)

Sample `video_2026-09-29_23-08-26.mp4`: character stationary while Whirligig Saw blade rotates, leaving trailing ghost artifacts. `BB_MOTION_SELECT_LOG=1` revealed root cause: the secondary vertex buffer was a 48-byte (3x4) bone palette, measuring 96–384 bytes (2–8 bones) for weapons and animated props. The prior ">= 640 bytes" threshold excluded these draws, causing rotating blades to receive static camera vectors.

Resolutions:
- `Motion::ClassifyBuffer` (`motion_history.h`): 864 = scene constants, >= 640 = character skeleton, 96..639 (multiples of 48) = small bone palettes. Pipelines with bone palettes now generate motion vector variants.
- Small bone palettes are gated: palette hashes are compared against the prior frame (`History::Moving`) before scanning indices. Stationary props use camera vectors; modified palettes track object motion.
- In-game debug overlay: "Show Motion Vectors (Debug)". Red/Green indicate |x|/|y| velocity magnitude; Blue indicates active object motion vector assignment.
