// SPDX-License-Identifier: GPL-2.0-or-later
#include "overlay_internal.h"
#include "overlay_helpers.h"

namespace BbOverlay {

void DrawUpscalerSection();
void DrawEffectsSection();

void DrawMenu() {
    bool open = menu_open.load();
    ImGui::SetNextWindowSize(ImVec2(480 * base_scale, 520 * base_scale), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Bloodborne PC Configuration", &open, ImGuiWindowFlags_None)) {
        ImGui::End();
        if (!open) SetOpen(false);
        return;
    }

    if (ImGui::BeginTabBar("ConfigTabs")) {
        if (ImGui::BeginTabItem("Upscaler & Graphics")) {
            DrawUpscalerSection();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Display & Effects")) {
            DrawEffectsSection();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();
    if (ImGui::Button("Close Menu", ImVec2(120 * base_scale, 0))) {
        open = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Configuration", ImVec2(150 * base_scale, 0))) {
        BbSettings::Save();
    }

    ImGui::End();
    if (!open) {
        SetOpen(false);
    }
}

} // namespace BbOverlay
