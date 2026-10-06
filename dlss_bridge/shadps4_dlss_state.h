// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: MIT
#pragma once

#include <array>
#include <cstdio>
#include <string>
#include "shadps4_dlss_bridge.h"

#include <nvsdk_ngx_helpers.h>
#include <nvsdk_ngx_vk.h>
#include <nvsdk_ngx_helpers_vk.h>

constexpr char DlssProjectId[] = "ea385843-a44c-4ea5-8e6d-46a16125aef0";
constexpr char DlssEngineVersion[] = "shadPS4-Bloodborne-DLSS";

struct DlssState {
    ShadDlssLogFn log{};
    std::wstring dll_directory;
    std::wstring data_directory;
    const wchar_t* dll_path{};
    NVSDK_NGX_FeatureCommonInfo common{};
    NVSDK_NGX_FeatureDiscoveryInfo discovery{};
    NVSDK_NGX_Parameter* capabilities{};
    NVSDK_NGX_Parameter* parameters{};
    NVSDK_NGX_Handle* feature{};
    ShadDlssFeature feature_desc{};
    VkDevice device{};
    bool configured{};
    bool initialized{};
};

extern DlssState g_dlss_state;

void DlssLog(int warning, const char* message);
bool DlssCheck(const char* operation, NVSDK_NGX_Result result);
void NVSDK_CONV DlssNgxLog(const char* message, NVSDK_NGX_Logging_Level, NVSDK_NGX_Feature);
