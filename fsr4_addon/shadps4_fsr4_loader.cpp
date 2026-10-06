// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: GPL-2.0-or-later
#include "shadps4_fsr4_state.h"

static bool LoadFile(const char* name, std::vector<uint8_t>& data) {
    if (Fsr4ReadFile(g_fsr4_state.directory / "opt" / name, data) ||
        Fsr4ReadFile(g_fsr4_state.directory / name, data))
        return true;
    Fsr4Fail(std::string{"FSR 4: model file "} + name + " is missing from the fsr4 folder");
    return false;
}

bool Fsr4LoadAssets(const ShadFsr4Context& c, FfxFsr4V07AssetSet& assets,
                    std::array<std::vector<uint8_t>, FFX_FSR4_VK_PASS_COUNT>& code,
                    std::vector<uint8_t>& initializer, std::vector<uint8_t>& weights) {
    const FfxFsr4ModelPreset preset = Fsr4PresetFor(c);
    if (!ffxFsr4V07BuildAssetSet(preset, c.output_width, c.output_height, &assets)) {
        Fsr4Fail("FSR 4: this output size is not supported by the v07 model");
        return false;
    }
    if (!LoadFile(assets.pre, code[0]))
        return false;
    for (uint32_t pass = 0; pass < FFX_FSR4_MODEL_PASS_COUNT; ++pass) {
        if (!LoadFile(assets.model[pass], code[1 + pass]))
            return false;
    }
    if (!LoadFile(assets.post, code[13]) || !LoadFile(assets.rcas, code[14]) ||
        !LoadFile(assets.spdAutoExposure, code[15]) ||
        !LoadFile(assets.initializer, initializer) ||
        !LoadFile(assets.prePassWeights, weights))
        return false;
    if (initializer.size() != FFX_FSR4_V07_INITIALIZER_BYTES ||
        weights.size() != FFX_FSR4_V07_PRE_PASS_WEIGHTS_BYTES) {
        Fsr4Fail("FSR 4: model weights have an unexpected size");
        return false;
    }
    return true;
}
