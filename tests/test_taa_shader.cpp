// SPDX-License-Identifier: GPL-2.0-or-later
#include "test_taa_common.h"
#include "test_taa_cases.h"
#include "test_taa_sharpen.h"

int main(int, char**) {
    Vulkan::Instance instance(0, false);
    static vk::detail::DynamicLoader loader;
    vk::detail::DispatchLoaderDynamic d;
    d.init(loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));
    d.init(instance.GetInstance()); d.init(instance.GetDevice());
    const auto device = instance.GetDevice(); Vulkan::Scheduler scheduler(instance);

    std::array<VideoCore::UniqueImage, 7> images; std::array<vk::UniqueImageView, 7> views;
    std::array<vk::DescriptorSetLayoutBinding, 7> bindings{};
    CreateTaaImages(device, instance.GetAllocator(), d, images, views, bindings);

    auto dsl = device.createDescriptorSetLayoutUnique({.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR, .bindingCount = u32(bindings.size()), .pBindings = bindings.data()}, nullptr, d).value;
    const vk::PushConstantRange push{vk::ShaderStageFlagBits::eCompute, 0, 64};
    auto layout = device.createPipelineLayoutUnique({.setLayoutCount = 1, .pSetLayouts = &*dsl, .pushConstantRangeCount = 1, .pPushConstantRanges = &push}, nullptr, d).value;
    auto module = device.createShaderModuleUnique({.codeSize = sizeof(TAA_COMP), .pCode = TAA_COMP}, nullptr, d).value;
    auto pipeline = device.createComputePipelineUnique({}, {.stage = {.stage = vk::ShaderStageFlagBits::eCompute, .module = *module, .pName = "main"}, .layout = *layout}, nullptr, d).value;
    auto sampler = device.createSamplerUnique({.magFilter = vk::Filter::eNearest, .minFilter = vk::Filter::eNearest, .mipmapMode = vk::SamplerMipmapMode::eNearest, .addressModeU = vk::SamplerAddressMode::eClampToEdge, .addressModeV = vk::SamplerAddressMode::eClampToEdge, .addressModeW = vk::SamplerAddressMode::eClampToEdge}, nullptr, d).value;

    VkBuffer staging{}; VmaAllocation alloc{}; VmaAllocationInfo ai{};
    const VkBufferCreateInfo bi{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = N * 16 * 4, .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    const VmaAllocationCreateInfo ac{.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT, .usage = VMA_MEMORY_USAGE_AUTO, .requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    assert(vmaCreateBuffer(instance.GetAllocator(), &bi, &ac, &staging, &alloc, &ai) == VK_SUCCESS);

    auto run = [&](const char* lbl, u32 reset, float hist, float hdepth, float mot, float hi, float exp, bool psn = false, float sdepth = 0.5f, float z_tr = 0.f, float j_x = 0.f, bool edge = false, float dslope = 0.f, bool thin_e = false, bool h_e = false, bool thin_h = false) {
        auto* px = static_cast<float*>(ai.pMappedData);
        for (u32 img = 0; img < 4; ++img) {
            for (u32 p = 0; p < N; ++p) {
                float* c = px + img * N * 4 + p * 4;
                c[0] = c[1] = c[2] = (img == 0) ? (p == center ? 0.25f : hi) : (img == 1) ? sdepth : (img == 2) ? 0.f : hist;
                c[3] = (img == 3) ? hdepth : 1.f;
                if (img == 2) c[0] = mot;
                if (img == 1) c[0] = std::clamp(sdepth + (int(p % W) - int(X)) * dslope, 0.f, 1.f);
                if (img == 1 && thin_e && p % W == X - 1) c[0] = 0.99f;
                if (img == 3 && psn) c[0] = c[1] = c[2] = std::numeric_limits<float>::quiet_NaN();
                if (img == 3 && edge && p % W == X + 1) c[3] = 0.75f;
                if (img == 3 && thin_h && p % W != X) c[3] = sdepth;
                if (img == 3 && h_e && p % W != X) c[0] = c[1] = c[2] = 0.25f;
            }
        }
        px[(center - 1) * 4] = px[(center - 1) * 4 + 1] = px[(center - 1) * 4 + 2] = 0.f;
        auto cmd = scheduler.CommandBuffer();
        std::array<vk::ImageMemoryBarrier2, 7> tr{};
        for (u32 i = 0; i < 7; ++i) tr[i] = {.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands, .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands, .dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite, .oldLayout = vk::ImageLayout::eUndefined, .newLayout = vk::ImageLayout::eGeneral, .image = vk::Image(images[i]), .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
        cmd.pipelineBarrier2({.imageMemoryBarrierCount = 7, .pImageMemoryBarriers = tr.data()}, d);
        for (u32 i = 0; i < 4; ++i) cmd.copyBufferToImage(staging, vk::Image(images[i]), vk::ImageLayout::eGeneral, vk::BufferImageCopy{.bufferOffset = i * N * 16, .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, .imageExtent = {W, H, 1}}, d);
        TaaBarrier(cmd, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite, vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderRead, d);
        std::array<vk::DescriptorImageInfo, 7> infos{}; std::array<vk::WriteDescriptorSet, 7> wr{};
        for (u32 i = 0; i < 7; ++i) { infos[i] = {.sampler = i < 4 ? *sampler : vk::Sampler{}, .imageView = *views[i], .imageLayout = vk::ImageLayout::eGeneral}; wr[i] = {.dstBinding = i, .descriptorCount = 1, .descriptorType = bindings[i].descriptorType, .pImageInfo = &infos[i]}; }
        struct P { float x, y; u32 rst, pad; std::array<float, 4> pr, ppr, pz; } params{j_x, 0, reset, 0, {1, 1, 1, -0.05f}, {1, 1, 1, -0.05f}, {0, 0, 1, z_tr}};
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline, d);
        cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eCompute, *layout, 0, wr, d);
        cmd.pushConstants(*layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(params), &params, d);
        cmd.dispatch((W + 7) / 8, (H + 7) / 8, 1, d);
        TaaBarrier(cmd, vk::PipelineStageFlagBits2::eComputeShader, vk::AccessFlagBits2::eShaderWrite, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead, d);
        for (u32 i = 4; i < 6; ++i) cmd.copyImageToBuffer(vk::Image(images[i]), vk::ImageLayout::eGeneral, staging, vk::BufferImageCopy{.bufferOffset = (i - 4) * 16, .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, .imageOffset = {s32(X), 3, 0}, .imageExtent = {1, 1, 1}}, d);
        TaaBarrier(cmd, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferWrite, vk::PipelineStageFlagBits2::eHost, vk::AccessFlagBits2::eHostRead, d);
        scheduler.Finish();
        const auto* res = static_cast<u16*>(ai.pMappedData);
        assert(std::abs(TaaHalf(res[0]) - exp) < 0.001f);
        std::printf("TAA shader: %s PASS (%.3f)\n", lbl, TaaHalf(res[0]));
    };

    RunTaaCases(run);
    RunTaaSharpen(scheduler, device, d, images, views, staging, ai);
    vmaDestroyBuffer(instance.GetAllocator(), staging, alloc);
}
