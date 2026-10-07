// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

namespace BbSettings {

enum Upscaler : int {
    UpscalerOff = 0, UpscalerFsr3 = 1, UpscalerFsr4 = 2, UpscalerFsr411 = 3,
    UpscalerTaa = 4, UpscalerCount
};

inline bool IsFsr4(int upscaler) {
    return upscaler == UpscalerFsr4 || upscaler == UpscalerFsr411;
}

enum Preset : int { NativeAA = 0, Quality, Balanced, Performance, UltraPerformance, PresetCount };
enum DebugView : int { DebugNone = 0, DebugReactive = 1, DebugMotion = 2, DebugViewCount };

struct Effect {
    const char* key;
    const char* label;
    bool default_on;
};

inline constexpr Effect Effects[] = {
    {"effect_chromatic_aberration", "Chromatic Aberration", true},
    {"effect_dof", "Depth of Field (DoF)", true},
    {"effect_motion_blur", "Motion Blur", true},
    {"effect_ssao", "Ambient Occlusion (SSAO)", true},
    {"effect_game_aa", "Game Native Anti-Aliasing", true},
    {"effect_dynamic_shadows", "Dynamic Point Shadows", true},
    {"effect_ssr", "Screen-Space Reflections (SSR)", false},
    {"skip_intro", "Skip Intro Cinematics", false},
    {"debug_camera", "Free Camera (Cross + L3)", false},
    {"debug_menu", "Debug Menu (requires font files)", false},
};

inline constexpr int EffectCount = int(sizeof(Effects) / sizeof(Effects[0]));
inline constexpr int OutputWidths[] = {1280, 1920, 2560, 3840};
inline constexpr int OutputHeights[] = {720, 1080, 1440, 2160};
inline constexpr int OutputCount = 4;
inline constexpr int OutputDefault = 1;

float PresetScale(int preset);
const char* PresetName(int preset);
const char* UpscalerName(int upscaler);

} // namespace BbSettings
