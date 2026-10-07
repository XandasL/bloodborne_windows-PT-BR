// SPDX-License-Identifier: GPL-2.0-or-later
#include <cstdio>
#include "test_camera_motion_pipeline.h"

int main() {
    Vulkan::Instance instance(0, false);
    static vk::detail::DynamicLoader loader;
    vk::detail::DispatchLoaderDynamic d;
    d.init(loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));
    d.init(instance.GetInstance()); d.init(instance.GetDevice());
    const auto device = instance.GetDevice();
    Vulkan::Scheduler scheduler(instance);
    std::array<VideoCore::UniqueImage, 3> images;
    std::array<vk::UniqueImageView, 3> views;
    std::array<vk::DescriptorSetLayoutBinding, 3> bindings{};
    InitMotionImages(device, instance.GetAllocator(), d, images, views, bindings);

    auto dsl = device.createDescriptorSetLayoutUnique({.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR,
        .bindingCount = 3, .pBindings = bindings.data()}, nullptr, d).value;
    const vk::PushConstantRange push{vk::ShaderStageFlagBits::eCompute, 0, sizeof(Params)};
    auto layout = device.createPipelineLayoutUnique({.setLayoutCount = 1, .pSetLayouts = &*dsl,
        .pushConstantRangeCount = 1, .pPushConstantRanges = &push}, nullptr, d).value;
    auto module = device.createShaderModuleUnique({.codeSize = sizeof(CAMERA_MOTION_COMP), .pCode = CAMERA_MOTION_COMP}, nullptr, d).value;
    auto pipeline = device.createComputePipelineUnique({}, {.stage = {.stage = vk::ShaderStageFlagBits::eCompute,
        .module = *module, .pName = "main"}, .layout = *layout}, nullptr, d).value;

    VkBuffer staging{}; VmaAllocation alloc{}; VmaAllocationInfo ai{};
    const VkBufferCreateInfo bi{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = N * 16 * 2,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    const VmaAllocationCreateInfo ac{.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO, .requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    assert(vmaCreateBuffer(instance.GetAllocator(), &bi, &ac, &staging, &alloc, &ai) == VK_SUCCESS);

    auto run = [&](const char* lbl, float depth, float trans, float rot, float odepth, u32 mode, float exp, bool jit = false) {
        auto* px = static_cast<float*>(ai.pMappedData);
        for (u32 p = 0; p < N; ++p) {
            px[p * 4] = depth; px[N * 4 + p * 4] = 2.f; px[N * 4 + p * 4 + 1] = 0.f;
            px[N * 4 + p * 4 + 2] = 1.f; px[N * 4 + p * 4 + 3] = odepth;
        }
        auto cmd = scheduler.CommandBuffer();
        std::array<vk::ImageMemoryBarrier2, 3> t{};
        for (u32 i = 0; i < 3; ++i) t[i] = {.srcStageMask = vk::PipelineStageFlagBits2::eAllCommands, .dstStageMask = vk::PipelineStageFlagBits2::eAllCommands,
            .dstAccessMask = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite, .oldLayout = vk::ImageLayout::eUndefined,
            .newLayout = vk::ImageLayout::eGeneral, .image = vk::Image(images[i]), .subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}};
        cmd.pipelineBarrier2({.imageMemoryBarrierCount = 3, .pImageMemoryBarriers = t.data()}, d);
        for (u32 i : {0u, 2u}) {
            const vk::BufferImageCopy r{.bufferOffset = i / 2 * N * 16, .imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, .imageExtent = {W, H, 1}};
            cmd.copyBufferToImage(staging, vk::Image(images[i]), vk::ImageLayout::eGeneral, r, d);
        }
        std::array<vk::DescriptorImageInfo, 3> info{}; std::array<vk::WriteDescriptorSet, 3> wr{};
        for (u32 i = 0; i < 3; ++i) { info[i] = {.imageView = *views[i], .imageLayout = vk::ImageLayout::eGeneral};
            wr[i] = {.dstBinding = i, .descriptorCount = 1, .descriptorType = bindings[i].descriptorType, .pImageInfo = &info[i]}; }
        Params p{{1, 0, rot, trans, 0, 1, 0, 0, 0, 0, 1, 0}, {1, 1, 1, -0.05f}, {1, 1, 1, -0.05f}, {W, H}, {jit ? 0.37f : 0.f, jit ? -0.29f : 0.f}, {0, 0}, mode};
        cmd.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline, d);
        cmd.pushDescriptorSetKHR(vk::PipelineBindPoint::eCompute, *layout, 0, wr, d);
        cmd.pushConstants(*layout, vk::ShaderStageFlagBits::eCompute, 0, sizeof(p), &p, d);
        cmd.dispatch((W + 7) / 8, (H + 7) / 8, 1, d);
        const vk::BufferImageCopy reg{.imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1}, .imageOffset = {4, 3, 0}, .imageExtent = {1, 1, 1}};
        cmd.copyImageToBuffer(vk::Image(images[1]), vk::ImageLayout::eGeneral, staging, reg, d);
        scheduler.Finish();
        const auto* res = static_cast<u16*>(ai.pMappedData);
        auto half = [](u16 v) { int e = (v >> 10) & 31; return (v & 0x8000 ? -1.f : 1.f) * (e == 0 ? std::ldexp(float(v & 1023), -24) : std::ldexp(1.f + float(v & 1023) / 1024.f, e - 15)); };
        assert(std::abs(half(res[0]) - exp) < 0.002f && std::abs(half(res[1])) < 0.001f);
        std::printf("Camera motion: %s PASS (%.5f)\n", lbl, half(res[0]));
    };

    constexpr float dst = 0.999995f;
    run("distant geometry moves", dst, 1000, 0, 0, 0, 4.5f * 1000 * (1 - dst) / 0.05f);
    run("sky rotates", 1, 0, 0.1f, 0, 0, 0.45f); run("sky ignores translation", 1, 1000, 0, 0, 0, 0);
    run("static camera cancels jitter", dst, 0, 0, 0, 0, 0, true);
    run("matching object vector", dst, 0, 0, dst, 1, 2); run("stale distant object vector", dst, 0, 0, 0.9999f, 1, 0);
    vmaDestroyBuffer(instance.GetAllocator(), staging, alloc);
}
