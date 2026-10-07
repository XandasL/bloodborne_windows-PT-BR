// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include "common/slot_vector.h"
#include "video_core/renderer_vulkan/vk_instance.h"
#include "video_core/renderer_vulkan/vk_runtime.h"
#include "video_core/renderer_vulkan/vk_scheduler.h"
#include "video_core/renderer_vulkan/vk_scene_resolution.h"
#include "video_core/texture_cache/blit_helper.h"
#include <vk_mem_alloc.h>

using namespace Vulkan;

inline u32 ReadbackPixel(Scheduler& scheduler, vk::detail::DispatchLoaderDynamic& dispatch,
                         VkBuffer readback, void* pMappedData, vk::Image image,
                         vk::ImageAspectFlagBits aspect, vk::ImageLayout layout, u32 w, u32 h, u32 mip = 0) {
    scheduler.EndRendering();
    const auto cmd = scheduler.CommandBuffer();
    const vk::MemoryBarrier2 b{.srcStageMask=vk::PipelineStageFlagBits2::eAllCommands,
        .srcAccessMask=vk::AccessFlagBits2::eMemoryWrite, .dstStageMask=vk::PipelineStageFlagBits2::eTransfer,
        .dstAccessMask=vk::AccessFlagBits2::eTransferRead};
    cmd.pipelineBarrier2({.memoryBarrierCount=1, .pMemoryBarriers=&b}, dispatch);
    const vk::BufferImageCopy region{.imageSubresource={aspect, mip, 0, 1},
        .imageOffset={s32(w - 1), s32(h - 1), 0}, .imageExtent={1, 1, 1}};
    cmd.copyImageToBuffer(image, layout, readback, region, dispatch);
    const vk::MemoryBarrier2 host{.srcStageMask=vk::PipelineStageFlagBits2::eTransfer,
        .srcAccessMask=vk::AccessFlagBits2::eTransferWrite, .dstStageMask=vk::PipelineStageFlagBits2::eHost,
        .dstAccessMask=vk::AccessFlagBits2::eHostRead};
    cmd.pipelineBarrier2({.memoryBarrierCount=1, .pMemoryBarriers=&host}, dispatch);
    scheduler.Finish();
    u32 result = 0;
    std::memcpy(&result, pMappedData, 4);
    return result;
}

inline void ClearNative(Runtime& runtime, Scheduler& scheduler, vk::detail::DispatchLoaderDynamic& dispatch,
                        VideoCore::Image& color, VideoCore::Image& depth, u32 rgba, u32 stencil = 7) {
    vk::ClearValue clear{};
    clear.color.float32 = std::array{float(rgba & 255)/255, float((rgba>>8)&255)/255, float((rgba>>16)&255)/255, 1.0f};
    runtime.ClearImage(&color, {}, clear);
    runtime.Transit(&depth, vk::ImageLayout::eTransferDstOptimal, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite);
    runtime.FlushBarriers();
    scheduler.CommandBuffer().clearDepthStencilImage(depth.GetImage(), vk::ImageLayout::eTransferDstOptimal,
        vk::ClearDepthStencilValue{0.25f, stencil}, vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1}, dispatch);
}
