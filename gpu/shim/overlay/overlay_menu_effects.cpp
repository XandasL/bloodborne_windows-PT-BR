// SPDX-License-Identifier: GPL-2.0-or-later
#include "overlay_internal.h"
#include "overlay_helpers.h"

namespace BbOverlay {

void DrawEffectsSection() {
    auto& s = BbSettings::Get();
    ImGui::SeparatorText("Display & Output Resolution");
    static const char* res_names[] = {"1280x720 (Steam Deck)", "1920x1080 (FHD)", "2560x1440 (2K)", "3840x2160 (4K)"};
    int current_res = s.output_res;
    if (ImGui::BeginCombo("Output Resolution", res_names[current_res])) {
        for (int i = 0; i < BbSettings::OutputCount; ++i) {
            if (ImGui::Selectable(res_names[i], i == current_res)) Store(s.output_res, i, true);
        }
        ImGui::EndCombo();
    }
    Checkbox("Show FPS Counter", s.show_fps);

    ImGui::SeparatorText("Game Effects & Patches");
    for (int e = 0; e < BbSettings::EffectCount; ++e) {
        Checkbox(BbSettings::Effects[e].label, s.effects[e]);
    }

    if (BbSettings::ResolutionNeedsRestart()) {
        ImGui::Separator();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Resolution/preset change requires restarting the game.");
    }
}

} // namespace BbOverlay
