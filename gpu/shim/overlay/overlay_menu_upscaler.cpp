// SPDX-License-Identifier: GPL-2.0-or-later
#include "overlay_internal.h"
#include "overlay_helpers.h"

namespace BbOverlay {

void DrawUpscalerSection() {
    auto& s = BbSettings::Get();
    ImGui::SeparatorText("Temporal Upscaler");
    static const char* upscalers[] = {"Off", "FSR 3.1", "FSR 4 (INT8)", "FSR 4.1.1 (INT8)", "TAA (Native AA)"};
    int upscaler = s.upscaler;
    if (ImGui::BeginCombo("Upscaler", upscalers[upscaler])) {
        for (int i = 0; i < BbSettings::UpscalerCount; ++i) {
            const bool supported = i == BbSettings::UpscalerFsr4 ? s.fsr4_supported.load()
                : i == BbSettings::UpscalerFsr411 ? s.fsr411_supported.load() : true;
            ImGui::BeginDisabled(!supported);
            if (ImGui::Selectable(upscalers[i], i == upscaler)) Store(s.upscaler, i, true);
            ImGui::EndDisabled();
            if (!supported) {
                ImGui::SameLine();
                ImGui::TextDisabled("— unsupported by GPU");
            }
        }
        ImGui::EndCombo();
    }
    if (const char* problem = s.fsr4_problem.load()) {
        ImGui::PushTextWrapPos();
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.3f, 1.0f), "FSR 4 unavailable: %s", problem);
        ImGui::PopTextWrapPos();
    }
    if (BbSettings::IsFsr4(s.upscaler)) {
        Checkbox("FSR 4: Auto-Exposure", s.fsr4_auto_exposure);
        Checkbox("FSR 4: Invert Jitter Sign", s.fsr4_invert_jitter);
    }
    static const char* presets[] = {"Native AA (1.0x)", "Quality (1.5x)", "Balanced (1.7x)", "Performance (2.0x)", "Ultra (3.0x)"};
    int p = s.preset;
    if (ImGui::BeginCombo("Preset", presets[p])) {
        for (int i = 0; i < BbSettings::PresetCount; ++i) {
            if (ImGui::Selectable(presets[i], i == p)) Store(s.preset, i, true);
        }
        ImGui::EndCombo();
    }
    Checkbox("RCAS Sharpening", s.sharpen);
    if (s.sharpen) Slider("Sharpness", s.sharpness, 0.0f, 2.0f);
}

} // namespace BbOverlay
