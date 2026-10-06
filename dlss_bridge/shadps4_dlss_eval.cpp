// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: MIT
#include "shadps4_dlss_state.h"

int32_t DlssConfigure(const wchar_t*, const wchar_t*, ShadDlssLogFn);
int32_t DlssInstanceExtensions(uint32_t*, const VkExtensionProperties**);
int32_t DlssDeviceExtensions(VkInstance, VkPhysicalDevice, uint32_t*, const VkExtensionProperties**);
int32_t DlssInitialize(VkInstance, VkPhysicalDevice, VkDevice, PFN_vkGetInstanceProcAddr, PFN_vkGetDeviceProcAddr);
int32_t DlssCreateFeature(VkCommandBuffer, const ShadDlssFeature*);
void DlssReleaseFeature();

static NVSDK_NGX_Resource_VK Wrap(const ShadDlssImage& image, bool writable) {
    return NVSDK_NGX_Create_ImageView_Resource_VK(image.view, image.image, image.range,
                                                  image.format, image.width, image.height,
                                                  writable);
}

int32_t DlssEvaluate(VkCommandBuffer command, const ShadDlssEvaluate* evaluate) {
    if (!g_dlss_state.feature || !g_dlss_state.parameters || !evaluate)
        return 0;
    auto color = Wrap(evaluate->color, false);
    auto depth = Wrap(evaluate->depth, false);
    auto motion = Wrap(evaluate->motion, false);
    auto output = Wrap(evaluate->output, true);
    NVSDK_NGX_VK_DLSS_Eval_Params params{};
    params.Feature.pInColor = &color;
    params.Feature.pInOutput = &output;
    params.pInDepth = &depth;
    params.pInMotionVectors = &motion;
    params.InRenderSubrectDimensions = {g_dlss_state.feature_desc.input_width,
                                        g_dlss_state.feature_desc.input_height};
    params.InJitterOffsetX = evaluate->jitter_x;
    params.InJitterOffsetY = evaluate->jitter_y;
    params.InReset = evaluate->reset;
    params.InMVScaleX = params.InMVScaleY = 1.0f;
    params.InPreExposure = params.InExposureScale = 1.0f;
    params.InFrameTimeDeltaInMsec = evaluate->frame_ms;
    return DlssCheck("DLSS evaluation", NGX_VULKAN_EVALUATE_DLSS_EXT(
                                            command, g_dlss_state.feature,
                                            g_dlss_state.parameters, &params))
               ? 1
               : 0;
}

void DlssShutdown() {
    DlssReleaseFeature();
    if (g_dlss_state.capabilities) {
        NVSDK_NGX_VULKAN_DestroyParameters(g_dlss_state.capabilities);
        g_dlss_state.capabilities = nullptr;
    }
    if (g_dlss_state.initialized) {
        NVSDK_NGX_VULKAN_Shutdown1(g_dlss_state.device);
        g_dlss_state.initialized = false;
        g_dlss_state.device = VK_NULL_HANDLE;
    }
}

static const ShadDlssApi g_dlss_api{SHADPS4_DLSS_BRIDGE_ABI,
                                    DlssConfigure,
                                    DlssInstanceExtensions,
                                    DlssDeviceExtensions,
                                    DlssInitialize,
                                    DlssCreateFeature,
                                    DlssEvaluate,
                                    DlssReleaseFeature,
                                    DlssShutdown};

extern "C" __declspec(dllexport) const ShadDlssApi* ShadDlssGetApi() {
    return &g_dlss_api;
}
