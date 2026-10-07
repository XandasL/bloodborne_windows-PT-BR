// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "test_taa_common.h"

inline void RunTaaSharpen(Vulkan::Scheduler& scheduler, vk::Device device, vk::detail::DispatchLoaderDynamic& d,
                          std::array<VideoCore::UniqueImage, 7>& images, std::array<vk::UniqueImageView, 7>& views,
                          VkBuffer staging, VmaAllocationInfo& ai) {
    std::array<vk::DescriptorSetLayoutBinding, 2> b{};
    for (u32 i = 0; i < 2; ++i) b[i] = {.binding = i, .descriptorType = vk::DescriptorType::eStorageImage, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eCompute};
    auto desc = device.createDescriptorSetLayoutUnique({.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR, .bindingCount = 2, .pBindings = b.data()}, nullptr, d).value;
    const vk::PushConstantRange push{vk::ShaderStageFlagBits::eCompute, 0, 4};
    auto layout = device.createPipelineLayoutUnique({.setLayoutCount = 1, .pSetLayouts = &*desc, .pushConstantRangeCount = 1, .pPushConstantRanges = &push}, nullptr, d).value;
    auto module = device.createShaderModuleUnique({.codeSize = sizeof(TAA_SHARPEN_COMP), .pCode = TAA_SHARPEN_COMP}, nullptr, d).value;
    auto pipeline = device.createComputePipelineUnique({}, {.stage = {.stage = vk::ShaderStageFlagBits::eCompute, .module = *module, .pName = "main"}, .layout = *layout}, nullptr, d).value;

    auto sharpen = [&](const char* lbl, float str, float sc, bool flat) {
        auto* inp = static_cast<float*>(ai.pMappedData);
        for (u32 p = 0; p < N; ++p) { for (u32 c = 0; c < 3; ++c) inp[p * 4 + c] = (p == center && !flat ? 0.55f : 0.5f) * sc; inp[p * 4 + 3] = 0.99998f; }
        auto cmd = scheduler.CommandBuffer();
        TaaBarrier(cmd, vk::PipelineStageFlagBits2::eAllCommands, vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite, d);
        const vk::BufferImageCopy up{.imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, .imageExtent = {W, H, 1}};
        cmd.copyBufferToImage(staging, vk::Image(images[5]), vk::ImageLayout::eGeneral, up, d);
        TaaBarrier(cmd, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite, vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderStorageWrite, d);
        const std::array<vk::DescriptorImageInfo, 2> si{{{.imageView = *views[5], .imageLayout = vk::ImageLayout::eGeneral}, {.imageView = *views[4], .imageLayout = vk::ImageLayout::eGeneral}}};
        std::array<vk::WriteDescriptorSet, 2> sw{};
        for (u32 i = 0; i < 2; ++i) sw[i] = {.dstBinding = i, .descriptorCount = 1, .descriptorType = vk::DescriptorType::eStorageImage, .pImageInfo = &si[i]};
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline, d);
        cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eCompute, *layout, 0, sw, d);
        cmd.pushConstants(*layout, vk::ShaderStageFlagBits::eCompute, 0, 4, &str, d);
        cmd.dispatch((W + 7) / 8, (H + 7) / 8, 1, d);
        TaaBarrier(cmd, vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderStorageRead | vk::AccessFlagBits2::eShaderStorageWrite, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead, d);
        cmd.copyImageToBuffer(vk::Image(images[5]), vk::ImageLayout::eGeneral, staging, vk::BufferImageCopy{.bufferOffset = N * 16, .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, .imageExtent = {W, H, 1}}, d);
        cmd.copyImageToBuffer(vk::Image(images[4]), vk::ImageLayout::eGeneral, staging, vk::BufferImageCopy{.bufferOffset = N * 32, .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, .imageExtent = {W, H, 1}}, d);
        TaaBarrier(cmd, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite, vk::PipelineStageFlagBits2::eHost, vk::AccessFlagBits2::eHostRead, d);
        scheduler.Finish();
        const auto* out = reinterpret_cast<const u16*>(static_cast<const char*>(ai.pMappedData) + N * 32);
        for (u32 p = 0; p < N; ++p) {
            for (u32 c = 0; c < 3; ++c) { float val = TaaHalf(out[p * 4 + c]); assert(std::isfinite(val) && val >= 0 && val <= std::max(1.f, sc)); if (flat || str <= 0) assert(std::abs(val - inp[p * 4 + c]) < 0.003f); }
            assert(TaaHalf(out[p * 4 + 3]) == 1.f);
        }
        float val = TaaHalf(out[center * 4]);
        if (!flat && str > 0 && sc == 1) assert(val > inp[center * 4] + 0.002f);
        std::printf("TAA RCAS: %s PASS (%.4f), history unchanged\n", lbl, val);
        return val;
    };
    sharpen("zero bypass", 0, 1, false); sharpen("lower limit", -1, 1, false);
    float mid = sharpen("half strength", 0.5f, 1, false); float max_s = sharpen("full strength", 1, 1, false);
    assert(max_s > mid && sharpen("above one (extra pass for FSR)", 2, 1, false) > max_s);
    sharpen("flat gray", 1, 1, true); sharpen("flat black", 1, 0, true);
    sharpen("flat HDR", 1, 8, true); sharpen("HDR detail", 1, 8, false);
}
