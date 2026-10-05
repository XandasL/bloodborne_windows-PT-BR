# Changes & Fixes — 2026-09-27

## Implemented Fixes

### 1. Reactivity Mask Setting Persistence
- **File:** `gpu/shim/bbport_settings.cpp`
- **Issue:** The "Show Mask (Debug)" checkbox was not saved to `bbport.ini`.
- **Fix:** Added configuration load/store for `debug_view`.

### 2. Live Preset Switching on Radeon RX 7800 XT
- **Files:** 
  - `gpu/shadps4/video_core/renderer_vulkan/vk_temporal_upscaler.cpp`
  - `gpu/shim/bbport_overlay.cpp`
- **Issue:** RX 7800 XT does not support direct `blit D32S8`, previously requiring game restart on preset changes.
- **Fix:**
  - Fallback: UI depth buffer cleared rather than blitted.
  - Removed `BB_LIVE_SCALING_UNSUPPORTED` constraint.
  - Updated in-game overlay menu to allow dynamic preset selection without restart.

### 3. Jitter Default Configuration
- **File:** `bbport.ini`
- **Fix:** Verified `jitter=1` enabled by default. Optimized sharpness and mask parameters.

## Runtime Feature Behavior Without Restart

- **Preset switching** (Native AA / Quality / Balanced / Performance / Ultra Performance)
- Upscaler enable/disable toggle
- Sharpness adjustment
- Subpixel jitter toggle
- Reactivity mask toggle
- Debug visualization modes

**Game restart required only for:** Object motion vector pipeline configuration (`object_motion`).

## Modified Files

- `gpu/shim/bbport_settings.cpp`
- `gpu/shadps4/video_core/renderer_vulkan/vk_temporal_upscaler.cpp`
- `gpu/shim/bbport_overlay.cpp`
- `bbport.ini`
