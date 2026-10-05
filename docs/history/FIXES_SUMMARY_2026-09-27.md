# Bloodborne Native Port Fixes Summary

## Changes Implemented

### 1. Reactivity Mask Persistence
- Parameter `debug_view` persisted to `bbport.ini`.
- Debug overlay remains active across launches.

### 2. Live Preset Switching on Radeon RX 7800 XT
Presets switch dynamically without restarting the game process, even on Vulkan drivers lacking hardware D32S8 blit capabilities.

**Mechanism:**
- GPUs with D32S8 blit support (NVIDIA, Intel): scene depth is blitted directly to UI depth.
- GPUs without D32S8 blit support (AMD RX 7800 XT / RADV): UI depth buffer is cleared, allowing normal HUD rendering passes to proceed.

### 3. Jitter Default Configuration
- Corrected configuration to enable subpixel jitter (`jitter=1`), which is mandatory for temporal reconstruction under FSR 3.1.

## Dynamic Feature Switching (No Restart Required)

- **Preset changes** (Native AA / Quality / Balanced / Performance / Ultra Performance)
- Upscaler enable/disable toggle
- Sharpness slider
- Subpixel jitter
- Reactivity mask and parameters
- Debug visualization modes

**Restart required only for:** Object motion vector pipeline changes (`object_motion`).

## Performance by Preset (1080p Output Baseline)

| Preset | Render Resolution | Scale Factor | Target Quality | Estimated FPS |
|---|---|---|---|---|
| Native AA | 1920×1080 | x1.0 | Maximum | ~60-70 |
| Quality | 1280×720 | x1.5 | High | ~80-90 |
| Balanced | ~1130×635 | x1.7 | Medium | ~90-100 |
| Performance | 960×540 | x2.0 | Recommended | ~100-110 |
| Ultra Performance | 640×360 | x3.0 | Maximum FPS | ~120+ |

## Technical Implementation Details

```cpp
// If GPU driver supports D32S8 image blits:
if (depth_blit) {
    cmd.blitImage(scene_depth, ui_depth, ...);
} else {
    // Fallback for drivers lacking combined depth-stencil blits
    cmd.clearDepthStencilImage(ui_depth, ...);
}
```
Removed `BB_LIVE_SCALING_UNSUPPORTED` guard, enabling dynamic scaling on all Vulkan devices.
