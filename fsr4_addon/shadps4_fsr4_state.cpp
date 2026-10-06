// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: GPL-2.0-or-later
#include "shadps4_fsr4_state.h"
#include <fstream>

Fsr4State g_fsr4_state;

void Fsr4Log(bool warning, const std::string& message) {
    if (g_fsr4_state.log)
        g_fsr4_state.log(warning ? 1 : 0, message.c_str());
}

void Fsr4Fail(const std::string& reason) {
    g_fsr4_state.problem = reason;
    Fsr4Log(true, reason);
}

FfxFsr4ModelPreset Fsr4PresetFor(const ShadFsr4Context& c) {
    const auto preset = ffxFsr4SelectModelPreset(c.render_width, c.output_width, false);
    return preset == FFX_FSR4_MODEL_PRESET_PERFORMANCE ||
                   preset == FFX_FSR4_MODEL_PRESET_ULTRA_PERFORMANCE
               ? FFX_FSR4_MODEL_PRESET_BALANCED
               : preset;
}

bool Fsr4ReadFile(const std::filesystem::path& path, std::vector<uint8_t>& data) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        return false;
    data.resize(size_t(file.tellg()));
    file.seekg(0);
    return bool(file.read(reinterpret_cast<char*>(data.data()), std::streamsize(data.size())));
}

void Fsr4Release() {
    if (g_fsr4_state.context_ok)
        ffxFsr4V07DestroyContext(&g_fsr4_state.context, nullptr);
    if (g_fsr4_state.backend_ok)
        ffxFsr4VkDestroyContext(reinterpret_cast<FfxFsr4VkContext*>(g_fsr4_state.scratch.data()));
    g_fsr4_state.context = {};
    g_fsr4_state.context_ok = false;
    g_fsr4_state.backend = {};
    g_fsr4_state.backend_ok = false;
    g_fsr4_state.current.reset();
}
