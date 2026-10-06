// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <array>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>
#include "shadps4_fsr4.h"

extern "C" {
#include "ffx_vk_fsr4_v07.h"
#include "ffx_vk_fsr4_v07_assets.h"
#include "ffx_vk_fsr4_v07_schedule.h"
}

struct Fsr4State {
    ShadFsr4LogFn log{};
    std::filesystem::path directory;
    VkPhysicalDevice physical{};
    VkDevice device{};
    std::vector<uint8_t> scratch;
    FfxInterface backend{};
    bool backend_ok{};
    ffxContext context{};
    bool context_ok{};
    std::optional<ShadFsr4Context> current;
    FfxFsr4ModelPreset preset{};
    std::string problem;
};

extern Fsr4State g_fsr4_state;

void Fsr4Log(bool warning, const std::string& message);
void Fsr4Fail(const std::string& reason);
FfxFsr4ModelPreset Fsr4PresetFor(const ShadFsr4Context& c);
bool Fsr4ReadFile(const std::filesystem::path& path, std::vector<uint8_t>& data);
void Fsr4Release();

bool Fsr4LoadAssets(const ShadFsr4Context& c, FfxFsr4V07AssetSet& assets,
                    std::array<std::vector<uint8_t>, FFX_FSR4_VK_PASS_COUNT>& code,
                    std::vector<uint8_t>& initializer, std::vector<uint8_t>& weights);

int32_t Fsr4HasContext(const ShadFsr4Context* c);
int32_t Fsr4CreateContext(const ShadFsr4Context* c);
int32_t Fsr4Evaluate(VkCommandBuffer cmd, const ShadFsr4Evaluate* e, uint64_t frame_id);
