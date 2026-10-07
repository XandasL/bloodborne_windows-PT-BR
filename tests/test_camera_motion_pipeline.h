// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <array>
#include <cassert>
#include <cmath>
#include "video_core/renderer_vulkan/vk_instance.h"
#include "video_core/renderer_vulkan/vk_scheduler.h"
#include "video_core/texture_cache/image.h"
#include "video_core/host_shaders/camera_motion_comp.h"
#include <vk_mem_alloc.h>

constexpr u32 W = 9, H = 7, N = W * H;

struct Params {
    std::array<float, 12> reproject;
    std::array<float, 4> proj, prev_proj;
    std::array<float, 2> size, jitter, previous_jitter;
    u32 mode;
};

inline void InitMotionImages(vk::Device device, VmaAllocator alloc, vk::detail::DispatchLoaderDynamic& d,
                            std::array<VideoCore::UniqueImage, 3>& images,
                            std::array<vk::UniqueImageView, 3>& views,
                            std::array<vk::DescriptorSetLayoutBinding, 3>& bindings) {
    for (u32 i = 0; i < 3; ++i) {
        const auto format = (i == 1) ? vk::Format::eR16G16Sfloat : vk::Format::eR32G32B32A32Sfloat;
        images[i] = VideoCore::UniqueImage(device, alloc);
        images[i].Create({.imageType = vk::ImageType::e2D, .format = format, .extent = {W, H, 1},
            .mipLevels = 1, .arrayLayers = 1, .samples = vk::SampleCountFlagBits::e1,
            .tiling = vk::ImageTiling::eOptimal, .usage = vk::ImageUsageFlagBits::eSampled |
            vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eTransferSrc |
            vk::ImageUsageFlagBits::eTransferDst});
        views[i] = device.createImageViewUnique({.image = vk::Image(images[i]),
            .viewType = vk::ImageViewType::e2D, .format = format,
            .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}}, nullptr, d).value;
        bindings[i] = {.binding = i, .descriptorType = (i == 1) ? vk::DescriptorType::eStorageImage :
            vk::DescriptorType::eSampledImage, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eCompute};
    }
}
