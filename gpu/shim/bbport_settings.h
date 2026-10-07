// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <atomic>
#include "bbport_types.h"

namespace BbSettings {

struct Values {
    std::atomic<int> upscaler{UpscalerFsr3};
    std::atomic<int> preset{NativeAA};
    std::atomic<bool> sharpen{true};
    std::atomic<float> sharpness{0.3f};
    std::atomic<bool> jitter{true};
    std::atomic<bool> reactive{false};
    std::atomic<bool> object_motion{true};
    std::atomic<float> reactive_scale{1.0f};
    std::atomic<float> reactive_threshold{0.2f};
    std::atomic<float> reactive_max{0.9f};
    std::atomic<int> debug_view{DebugNone};
    std::atomic<bool> show_fps{false};
    std::atomic<bool> fsr4_auto_exposure{true};
    std::atomic<bool> fsr4_invert_jitter{false};
    std::atomic<int> active_render_width{1920}, active_render_height{1080};
    std::atomic<bool> effects[EffectCount]{};
    std::atomic<int> model_lod{0};
    std::atomic<int> output_res{OutputDefault};
    std::atomic<int> live_resolution{0};
    std::atomic<const char*> fsr4_problem{nullptr};
    std::atomic<bool> fsr4_supported{false}, fsr411_supported{false};

    int startup_preset = NativeAA;
    int startup_upscaler = UpscalerFsr3;
    bool startup_object_motion = true;
    bool startup_effects[EffectCount]{};
    int startup_model_lod = 0;
    int startup_output_res = OutputDefault;
    int startup_live_resolution = 0;
};

Values& Get();
void Load();
void ConfigureUpscalerSupport(bool fsr4, bool fsr411);
bool FixedRenderSession();
int RenderPreset();
bool ResolutionNeedsRestart();
void Save();

} // namespace BbSettings
