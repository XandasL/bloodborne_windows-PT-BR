// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: MIT
#include <algorithm>
#include "shadps4_dlss_state.h"

void DlssReleaseFeature() {
    if (g_dlss_state.feature) {
        NVSDK_NGX_VULKAN_ReleaseFeature(g_dlss_state.feature);
        g_dlss_state.feature = nullptr;
    }
    if (g_dlss_state.parameters) {
        NVSDK_NGX_VULKAN_DestroyParameters(g_dlss_state.parameters);
        g_dlss_state.parameters = nullptr;
    }
}

int32_t DlssCreateFeature(VkCommandBuffer command, const ShadDlssFeature* desc) {
    if (!g_dlss_state.initialized || !g_dlss_state.capabilities || !desc)
        return 0;
    DlssReleaseFeature();
    if (!DlssCheck("Parameter allocation",
                   NVSDK_NGX_VULKAN_AllocateParameters(&g_dlss_state.parameters)))
        return 0;
    for (const char* key : {NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_DLAA,
                            NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Quality,
                            NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Balanced,
                            NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_Performance,
                            NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraPerformance,
                            NVSDK_NGX_Parameter_DLSS_Hint_Render_Preset_UltraQuality}) {
        NVSDK_NGX_Parameter_SetUI(g_dlss_state.parameters, key, desc->preset);
    }
    static constexpr std::array qualities{
        NVSDK_NGX_PerfQuality_Value_DLAA, NVSDK_NGX_PerfQuality_Value_MaxQuality,
        NVSDK_NGX_PerfQuality_Value_Balanced, NVSDK_NGX_PerfQuality_Value_MaxPerf,
        NVSDK_NGX_PerfQuality_Value_UltraPerformance};
    NVSDK_NGX_DLSS_Create_Params settings{};
    settings.Feature.InWidth = desc->input_width;
    settings.Feature.InHeight = desc->input_height;
    settings.Feature.InTargetWidth = desc->output_width;
    settings.Feature.InTargetHeight = desc->output_height;
    settings.Feature.InPerfQualityValue = qualities[std::clamp(desc->quality, 0, 4)];
    settings.InFeatureCreateFlags =
        NVSDK_NGX_DLSS_Feature_Flags_MVLowRes |
        (desc->depth_inverted ? NVSDK_NGX_DLSS_Feature_Flags_DepthInverted : 0) |
        (desc->hdr ? NVSDK_NGX_DLSS_Feature_Flags_IsHDR | NVSDK_NGX_DLSS_Feature_Flags_AutoExposure
                   : 0);
    const auto result = NGX_VULKAN_CREATE_DLSS_EXT1(
        g_dlss_state.device, command, 1, 1, &g_dlss_state.feature, g_dlss_state.parameters, &settings);
    if (!DlssCheck("DLSS feature creation", result) || !g_dlss_state.feature) {
        g_dlss_state.feature = nullptr;
        return 0;
    }
    g_dlss_state.feature_desc = *desc;
    return 1;
}
