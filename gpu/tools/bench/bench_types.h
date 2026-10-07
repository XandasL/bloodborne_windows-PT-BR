// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "ffx_vk_fsr4_v07.h"
#include "ffx_vk_fsr4_v07_assets.h"
#include "fsr411.h"

#define CHECK(call) do { const VkResult r_ = (call); if (r_ != VK_SUCCESS) { std::fprintf(stderr, "%s failed: %d\n", #call, int(r_)); std::exit(1); } } while (0)

inline bool ReadFile(const std::string& path, std::vector<unsigned char>& data) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return false;
    data.resize(size_t(file.tellg())); file.seekg(0);
    return bool(file.read(reinterpret_cast<char*>(data.data()), std::streamsize(data.size())));
}

struct Gpu {
    VkInstance instance{}; VkPhysicalDevice physical{}; VkDevice device{};
    VkQueue queue{}; uint32_t family = 0; VkPhysicalDeviceMemoryProperties memory{};
};

struct Buffer {
    VkBuffer buffer{}; VkDeviceMemory memory{}; void* data = nullptr;
};

struct Image {
    VkImage image{}; VkImageView view{}; VkDeviceMemory memory{};
    uint32_t width = 0, height = 0;
};

inline uint16_t Half(float f) {
    uint32_t x; std::memcpy(&x, &f, 4);
    const uint32_t sign = (x >> 16) & 0x8000u;
    const int exp = int((x >> 23) & 0xffu) - 127 + 15;
    if (exp <= 0) return uint16_t(sign);
    return uint16_t(sign | (uint32_t(exp) << 10) | ((x >> 13) & 0x3ffu));
}

inline FfxFsr4ModelPreset ModelPreset(int preset) {
    switch (preset) {
    case 0: return FFX_FSR4_MODEL_PRESET_NATIVE_AA;
    case 1: return FFX_FSR4_MODEL_PRESET_QUALITY;
    case 2: return FFX_FSR4_MODEL_PRESET_BALANCED;
    case 3: return FFX_FSR4_MODEL_PRESET_PERFORMANCE;
    default: return FFX_FSR4_MODEL_PRESET_ULTRA_PERFORMANCE;
    }
}
