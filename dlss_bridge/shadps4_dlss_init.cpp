// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: MIT
#include "shadps4_dlss_state.h"

int32_t DlssConfigure(const wchar_t* dll_dir, const wchar_t* data_dir,
                      ShadDlssLogFn log) {
    g_dlss_state.log = log;
    g_dlss_state.dll_directory = dll_dir ? dll_dir : L".";
    g_dlss_state.data_directory = data_dir ? data_dir : L".";
    g_dlss_state.dll_path = g_dlss_state.dll_directory.c_str();
    g_dlss_state.common.PathListInfo = {&g_dlss_state.dll_path, 1};
    g_dlss_state.common.LoggingInfo = {DlssNgxLog, NVSDK_NGX_LOGGING_LEVEL_OFF, false};
    g_dlss_state.discovery.SDKVersion = NVSDK_NGX_Version_API;
    g_dlss_state.discovery.FeatureID = NVSDK_NGX_Feature_SuperSampling;
    g_dlss_state.discovery.Identifier.IdentifierType =
        NVSDK_NGX_Application_Identifier_Type_Project_Id;
    g_dlss_state.discovery.Identifier.v.ProjectDesc = {
        DlssProjectId, NVSDK_NGX_ENGINE_TYPE_CUSTOM, DlssEngineVersion};
    g_dlss_state.discovery.ApplicationDataPath = g_dlss_state.data_directory.c_str();
    g_dlss_state.discovery.FeatureInfo = &g_dlss_state.common;
    g_dlss_state.configured = true;
    return 1;
}

int32_t DlssInstanceExtensions(uint32_t* count, const VkExtensionProperties** extensions) {
    VkExtensionProperties* req{};
    if (!g_dlss_state.configured ||
        !DlssCheck("Instance extension query",
                   NVSDK_NGX_VULKAN_GetFeatureInstanceExtensionRequirements(
                       &g_dlss_state.discovery, count, &req)))
        return 0;
    *extensions = req;
    return 1;
}

int32_t DlssDeviceExtensions(VkInstance instance, VkPhysicalDevice physical, uint32_t* count,
                             const VkExtensionProperties** extensions) {
    if (!g_dlss_state.configured) return 0;
    NVSDK_NGX_FeatureRequirement support{};
    if (!DlssCheck("Feature requirement query",
                   NVSDK_NGX_VULKAN_GetFeatureRequirements(
                       instance, physical, &g_dlss_state.discovery, &support)) ||
        support.FeatureSupported != NVSDK_NGX_FeatureSupportResult_Supported)
        return 0;
    VkExtensionProperties* req{};
    if (!DlssCheck("Device extension query",
                   NVSDK_NGX_VULKAN_GetFeatureDeviceExtensionRequirements(
                       instance, physical, &g_dlss_state.discovery, count, &req)))
        return 0;
    *extensions = req;
    return 1;
}

int32_t DlssInitialize(VkInstance instance, VkPhysicalDevice physical, VkDevice device,
                       PFN_vkGetInstanceProcAddr get_inst, PFN_vkGetDeviceProcAddr get_dev) {
    if (!g_dlss_state.configured || g_dlss_state.initialized) return 0;
    if (!DlssCheck("NGX initialization",
                   NVSDK_NGX_VULKAN_Init_with_ProjectID(
                       DlssProjectId, NVSDK_NGX_ENGINE_TYPE_CUSTOM, DlssEngineVersion,
                       g_dlss_state.data_directory.c_str(), instance, physical, device,
                       get_inst, get_dev, &g_dlss_state.common)))
        return 0;
    g_dlss_state.initialized = true;
    g_dlss_state.device = device;
    if (!DlssCheck("Capability query",
                   NVSDK_NGX_VULKAN_GetCapabilityParameters(&g_dlss_state.capabilities)))
        return 0;
    int avail{}, needs_update{};
    NVSDK_NGX_Parameter_GetI(g_dlss_state.capabilities,
                             NVSDK_NGX_Parameter_SuperSampling_Available, &avail);
    NVSDK_NGX_Parameter_GetI(g_dlss_state.capabilities,
                             NVSDK_NGX_Parameter_SuperSampling_NeedsUpdatedDriver, &needs_update);
    return (!needs_update && avail) ? 1 : 0;
}
