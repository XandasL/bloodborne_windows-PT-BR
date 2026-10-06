// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: GPL-2.0-or-later
#include "shadps4_fsr4_state.h"

int32_t Fsr4HasContext(const ShadFsr4Context* c) {
    return g_fsr4_state.context_ok && g_fsr4_state.current && c &&
           g_fsr4_state.current->output_width == c->output_width &&
           g_fsr4_state.current->output_height == c->output_height &&
           g_fsr4_state.preset == Fsr4PresetFor(*c);
}

int32_t Fsr4CreateContext(const ShadFsr4Context* c) {
    Fsr4Release();
    if (!c || !g_fsr4_state.device) return 0;
    const FfxFsr4ModelPreset preset = Fsr4PresetFor(*c);
    FfxFsr4V07AssetSet assets{};
    std::array<std::vector<uint8_t>, FFX_FSR4_VK_PASS_COUNT> code;
    std::vector<uint8_t> initializer, weights;
    if (!Fsr4LoadAssets(*c, assets, code, initializer, weights)) return 0;

    static const std::array<std::string, FFX_FSR4_VK_PASS_COUNT> entries = [] {
        std::array<std::string, FFX_FSR4_VK_PASS_COUNT> names;
        names.fill("main");
        for (uint32_t p = 1; p <= FFX_FSR4_MODEL_PASS_COUNT; ++p)
            names[p] = "fsr4_model_v07_i8_pass" + std::to_string(p);
        return names;
    }();

    FfxFsr4VkCreateInfo info{};
    info.device = g_fsr4_state.device;
    info.physicalDevice = g_fsr4_state.physical;
    for (uint32_t i = 0; i < FFX_FSR4_VK_PASS_COUNT; ++i) {
        if (code[i].size() % 4) {
            Fsr4Fail("FSR 4: invalid shader file");
            return 0;
        }
        info.shaders[i] = {reinterpret_cast<const uint32_t*>(code[i].data()),
                           code[i].size(), entries[i].c_str()};
    }
    info.modelInitializer = initializer.data();
    info.modelInitializerSize = initializer.size();
    info.prePassWeights = weights.data();
    info.prePassWeightsSize = weights.size();
    g_fsr4_state.scratch.assign(ffxFsr4VkGetScratchMemorySize(), 0);
    info.scratchBuffer = g_fsr4_state.scratch.data();
    info.scratchBufferSize = g_fsr4_state.scratch.size();
    if (const VkResult res = ffxFsr4VkCreateContext(&info, &g_fsr4_state.backend); res != VK_SUCCESS) {
        g_fsr4_state.backend = {};
        Fsr4Fail("FSR 4: Vulkan backend creation failed (" + std::to_string(int(res)) + ")");
        return 0;
    }
    g_fsr4_state.backend_ok = true;

    ffxCreateContextDescUpscale desc{};
    desc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
    desc.maxRenderSize = {(c->output_width + 7) & ~7u, (c->output_height + 7) & ~7u};
    desc.maxUpscaleSize = desc.maxRenderSize;
    ffxFsr4V07SetBackendInterface(&g_fsr4_state.backend);
    const auto created = ffxFsr4V07CreateContext(&g_fsr4_state.context, &desc.header, nullptr);
    ffxFsr4V07SetBackendInterface(nullptr);
    if (created != FFX_API_RETURN_OK) {
        Fsr4Release();
        Fsr4Fail("FSR 4: context creation failed (" + std::to_string(created) + ")");
        return 0;
    }
    g_fsr4_state.context_ok = true;
    g_fsr4_state.current = *c;
    g_fsr4_state.preset = preset;
    g_fsr4_state.problem.clear();
    Fsr4Log(false, "FSR 4: context created successfully");
    return 1;
}
