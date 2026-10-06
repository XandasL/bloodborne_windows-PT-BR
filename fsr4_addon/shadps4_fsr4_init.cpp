// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: GPL-2.0-or-later
#include "shadps4_fsr4_state.h"

int32_t Fsr4Configure(const wchar_t* directory, ShadFsr4LogFn log) {
    g_fsr4_state.log = log;
    g_fsr4_state.directory = directory ? std::filesystem::path{directory}
                                       : std::filesystem::path{};
    return 1;
}

int32_t Fsr4Initialize(VkPhysicalDevice physical, VkDevice device) {
    g_fsr4_state.physical = physical;
    g_fsr4_state.device = device;
    return physical && device ? 1 : 0;
}

void Fsr4Retire(uint64_t completed_frame_id) {
    if (g_fsr4_state.backend_ok)
        ffxFsr4VkRetireFrame(&g_fsr4_state.backend, completed_frame_id);
}

void Fsr4ReleaseContext() {
    Fsr4Release();
}

const char* Fsr4Problem() {
    return g_fsr4_state.problem.c_str();
}

constexpr ShadFsr4Api Api{
    SHADPS4_FSR4_ABI,
    Fsr4Configure,
    Fsr4Initialize,
    Fsr4HasContext,
    Fsr4CreateContext,
    Fsr4Evaluate,
    Fsr4Retire,
    Fsr4ReleaseContext,
    Fsr4Problem
};

extern "C" __declspec(dllexport) const ShadFsr4Api* ShadFsr4GetApi() {
    return &Api;
}
