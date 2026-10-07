// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cassert>
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <fstream>
#include <vector>
#include "video_core/renderer_vulkan/vk_instance.h"
#include "video_core/renderer_vulkan/vk_scheduler.h"
#include "video_core/texture_cache/image.h"
#include "video_core/host_shaders/taa_comp.h"
#include "video_core/host_shaders/taa_sharpen_comp.h"
#include <vk_mem_alloc.h>

constexpr u32 W = 2049, H = 7, N = W * H, X = 2010, center = 3 * W + X;

inline float TaaHalf(u16 v) {
    const int exp = (v >> 10) & 31;
    const float mant = float(v & 1023);
    return (v & 0x8000 ? -1.f : 1.f) * (exp == 0 ? std::ldexp(mant, -24) : std::ldexp(1.f + mant / 1024.f, exp - 15));
}

inline void TaaBarrier(vk::CommandBuffer cmd, vk::PipelineStageFlags2 src, vk::AccessFlags2 src_acc,
                       vk::PipelineStageFlags2 dst, vk::AccessFlags2 dst_acc, vk::detail::DispatchLoaderDynamic& d) {
    const vk::MemoryBarrier2 b{.srcStageMask = src, .srcAccessMask = src_acc, .dstStageMask = dst, .dstAccessMask = dst_acc};
    cmd.pipelineBarrier2({.memoryBarrierCount = 1, .pMemoryBarriers = &b}, d);
}

inline void CreateTaaImages(vk::Device device, VmaAllocator alloc, vk::detail::DispatchLoaderDynamic& d,
                            std::array<VideoCore::UniqueImage, 7>& images, std::array<vk::UniqueImageView, 7>& views,
                            std::array<vk::DescriptorSetLayoutBinding, 7>& bindings) {
    for (u32 i = 0; i < images.size(); ++i) {
        const auto fmt = (i != 4 && i != 6) ? vk::Format::eR32G32B32A32Sfloat : vk::Format::eR16G16B16A16Sfloat;
        images[i] = VideoCore::UniqueImage(device, alloc);
        images[i].Create({.imageType = vk::ImageType::e2D, .format = fmt, .extent = {W,H,1}, .mipLevels = 1, .arrayLayers = 1,
            .samples = vk::SampleCountFlagBits::e1, .tiling = vk::ImageTiling::eOptimal,
            .usage = vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst});
        views[i] = device.createImageViewUnique({.image = vk::Image(images[i]), .viewType = vk::ImageViewType::e2D, .format = fmt, .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}}, nullptr, d).value;
        bindings[i] = {.binding = i, .descriptorType = i < 4 ? vk::DescriptorType::eCombinedImageSampler : vk::DescriptorType::eStorageImage, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eCompute};
    }
}
