// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "bench_image.h"

inline void PrepareInputs(const Gpu& gpu, VkCommandBuffer cmd, const Image& color, const Image& depth,
                          const Image& motion, const Image& output, uint32_t rw, uint32_t rh,
                          const std::function<void()>& submit) {
    const auto to_general = [&](const Image& image, VkImageAspectFlags aspect) {
        VkImageMemoryBarrier b{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, nullptr, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED,
            image.image, {aspect, 0, 1, 0, 1}};
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &b);
    };
    to_general(color, VK_IMAGE_ASPECT_COLOR_BIT); to_general(depth, VK_IMAGE_ASPECT_DEPTH_BIT);
    to_general(motion, VK_IMAGE_ASPECT_COLOR_BIT); to_general(output, VK_IMAGE_ASPECT_COLOR_BIT);

    const VkImageSubresourceRange col_range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    const VkClearColorValue grey{{0.4f, 0.35f, 0.3f, 1.0f}}, mv{{0.25f, -0.5f, 0.0f, 0.0f}};
    vkCmdClearColorImage(cmd, color.image, VK_IMAGE_LAYOUT_GENERAL, &grey, 1, &col_range);
    vkCmdClearColorImage(cmd, motion.image, VK_IMAGE_LAYOUT_GENERAL, &mv, 1, &col_range);
    const VkClearDepthStencilValue far_d{0.5f, 0};
    const VkImageSubresourceRange dep_range{VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1};
    vkCmdClearDepthStencilImage(cmd, depth.image, VK_IMAGE_LAYOUT_GENERAL, &far_d, 1, &dep_range);
    VkMemoryBarrier mb{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &mb, 0, nullptr, 0, nullptr);
    submit();

    const char* noise = std::getenv("BENCH_NOISE");
    if (noise && noise[0] == '1') {
        const VkDeviceSize c_bytes = VkDeviceSize(rw) * rh * 8, m_bytes = VkDeviceSize(rw) * rh * 4, d_bytes = VkDeviceSize(rw) * rh * 4;
        Buffer stg = CreateHostBuffer(gpu, c_bytes + m_bytes + d_bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
        uint32_t seed = 12345u;
        auto next = [&] { seed = seed * 1664525u + 1013904223u; return float(seed >> 8) / float(1u << 24); };
        auto* bytes = static_cast<unsigned char*>(stg.data);
        auto* c16 = reinterpret_cast<uint16_t*>(bytes);
        for (VkDeviceSize i = 0; i < VkDeviceSize(rw) * rh; ++i) {
            c16[i * 4 + 0] = Half(next() * 1.5f); c16[i * 4 + 1] = Half(next() * 1.5f);
            c16[i * 4 + 2] = Half(next() * 1.5f); c16[i * 4 + 3] = Half(1.0f);
        }
        auto* m16 = reinterpret_cast<uint16_t*>(bytes + c_bytes);
        for (VkDeviceSize i = 0; i < VkDeviceSize(rw) * rh * 2; ++i) m16[i] = Half(next() * 4.0f - 2.0f);
        auto* dep = reinterpret_cast<float*>(bytes + c_bytes + m_bytes);
        for (VkDeviceSize i = 0; i < VkDeviceSize(rw) * rh; ++i) dep[i] = 0.9f + next() * 0.1f;

        const VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        CHECK(vkBeginCommandBuffer(cmd, &begin));
        VkBufferImageCopy r{.bufferOffset = 0, .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, .imageExtent = {rw, rh, 1}};
        vkCmdCopyBufferToImage(cmd, stg.buffer, color.image, VK_IMAGE_LAYOUT_GENERAL, 1, &r);
        r.bufferOffset = c_bytes; vkCmdCopyBufferToImage(cmd, stg.buffer, motion.image, VK_IMAGE_LAYOUT_GENERAL, 1, &r);
        r.bufferOffset = c_bytes + m_bytes; r.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        vkCmdCopyBufferToImage(cmd, stg.buffer, depth.image, VK_IMAGE_LAYOUT_GENERAL, 1, &r);
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 1, &mb, 0, nullptr, 0, nullptr);
        submit();
    }
}
