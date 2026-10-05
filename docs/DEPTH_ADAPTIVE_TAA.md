# Depth-Adaptive TAA Improvements

**Date:** 2026-10-01  
**Status:** Historical experiment, superseded by temporal reprojection overhaul.

The thresholds and weighting schemes described below are preserved for reference. Reducing history weight at far depth increased shimmering; device depth comparisons across different camera projections and storing depth in FP16 failed to reliably separate distant surfaces. For current implementation and verification results, see [TEMPORAL_REVIEW_2026-10-01.md](TEMPORAL_REVIEW_2026-10-01.md).

## Problem

Early TAA implementations used static thresholds for:
1. Disocclusion detection (revealing newly exposed geometry): `0.0005`
2. Temporal accumulation weighting: `0.9` → `0.65` depending on motion velocity

Fixed parameters did not account for non-linear depth distribution:
- **Distant geometry (depth → 1.0)**: Temporal instability, shimmering
- **Foreground geometry (depth → 0.0)**: Excessive ghosting / smearing during rapid camera movements

## Solution: Depth-Adaptive Thresholds

### 1. Depth-Adaptive Disocclusion Threshold

**File:** `gpu/shadps4/video_core/host_shaders/taa.comp`

```glsl
float depthThreshold(float depth) {
    // Near: 0.001, Far (>0.95): 0.0002
    return mix(0.001, 0.0002, smoothstep(0.7, 0.95, depth));
}
```

**Logic:**
- Foreground surfaces (depth < 0.7): `0.001` threshold accommodates minor variance
- Distant surfaces (depth > 0.95): `0.0002` threshold prevents false positives in compressed depth space
- Smooth transition via `smoothstep(0.7, 0.95, depth)`

**Usage:**
```glsl
float depthDiff = abs(history.a - depth);
float threshold = depthThreshold(depth);
if (depthDiff < threshold && ...) {
    // Accept history
}
```

### 2. Depth-Adaptive Temporal Weight

```glsl
float temporalWeight(float depth, float motionLength) {
    // Base weight decreases for distant objects: 0.9 near, 0.8 far
    float baseWeight = mix(0.9, 0.8, smoothstep(0.7, 0.95, depth));
    // Motion reduces weight more aggressively for distant pixels
    float motionFactor = mix(0.65, 0.5, smoothstep(0.7, 0.95, depth));
    return mix(baseWeight, motionFactor, clamp(motionLength / 16.0, 0.0, 1.0));
}
```

**Logic:**
- **Static Base Weight:**
  - Foreground: `0.9` (aggressive history accumulation)
  - Distant: `0.8` (conservative, reduced shimmering)
- **High Motion (motionLength > 16px):**
  - Foreground: scales down to `0.65`
  - Distant: scales down to `0.5`

### 3. Motion Vector Validation (camera_motion.comp)

**File:** `gpu/shadps4/video_core/host_shaders/camera_motion.comp`

```glsl
// Depth-adaptive validation: distant surfaces need tighter depth matching.
float depth_threshold = mix(0.001, 0.0003, smoothstep(0.7, 0.95, depth));
if (object_motion.b > 0.99 && abs(object_motion.a - depth) < depth_threshold &&
    !any(isnan(object_motion.rg)) && !any(isinf(object_motion.rg))) {
    result = object_motion.rg;
}
```

Prevents incorrect object motion vector application across depth discontinuities.

### 4. Debug Visualization Modes

**File:** `gpu/shadps4/video_core/host_shaders/camera_motion.comp`

`BB_DEBUG_MOTION=1` inspection modes:
```glsl
if (mode == 1) {
    // Raw depth visualization
    imageStore(color_img, pixel, vec4(fract(depth * 4.0), depth, depth < 0.0 ? 1.0 : 0.0, 1.0));
} else if (mode == 2) {
    // View-space depth visualization (z / 50)
    const float v = z / 50.0;
    imageStore(color_img, pixel, vec4(fract(v * 4.0), v, v < 0.0 ? 1.0 : 0.0, 1.0));
}
```

## References

- [upscaler.md](upscaler.md) — Temporal upscaler architecture
- [motion_vectors.md](motion_vectors.md) — Motion vector generation
- TAA shader: `gpu/shadps4/video_core/host_shaders/taa.comp`
- Camera motion: `gpu/shadps4/video_core/host_shaders/camera_motion.comp`
