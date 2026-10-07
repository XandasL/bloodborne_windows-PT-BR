// SPDX-License-Identifier: GPL-2.0-or-later
#include "test_scene_resolution_setup.h"
#include "test_scene_resolution_bloom.h"

int main() {
    Instance instance(0, false);
    static vk::detail::DynamicLoader loader;
    vk::detail::DispatchLoaderDynamic dispatch;
    dispatch.init(loader.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr"));
    dispatch.init(instance.GetInstance()); dispatch.init(instance.GetDevice());
    Scheduler scheduler(instance); Runtime runtime(instance, scheduler);
    Common::SlotVector<VideoCore::ImageView> views; Common::SlotVector<VideoCore::Image> images;
    VideoCore::ImageInfo ci{.size={1920,1080,1}, .resources={1,1}, .type=AmdGpu::ImageType::Color2D, .pixel_format=vk::Format::eR8G8B8A8Unorm, .num_bits=32};
    const auto color_id = images.insert(instance, runtime, views, ci);
    auto di = ci; di.pixel_format = vk::Format::eD32SfloatS8Uint; di.props.is_depth = di.props.has_stencil = true;
    const auto depth_id = images.insert(instance, runtime, views, di);
    SceneTargets targets(instance, scheduler, runtime, [&](VideoCore::ImageId id, u64 uid) {
        return (images.is_allocated(id) && (!uid || images[id].image_uid == uid)) ? &images[id] : nullptr;
    });
    if (!targets.Eligible(images[color_id]) || !targets.Eligible(images[depth_id])) return 0;

    VkBuffer readback{}; VmaAllocation alloc{}; VmaAllocationInfo ai{};
    const VkBufferCreateInfo bi{.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size=16, .usage=VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    const VmaAllocationCreateInfo ac{.flags=VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT, .usage=VMA_MEMORY_USAGE_AUTO, .requiredFlags=VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
    assert(vmaCreateBuffer(instance.GetAllocator(), &bi, &ac, &readback, &alloc, &ai) == VK_SUCCESS);
    VideoCore::ImageViewInfo cv{.format = ci.pixel_format}; auto dv = cv; dv.format = di.pixel_format;

    constexpr u32 stencils[] = {0, 255, 128, 85, 170, 7, 13, 255, 0, 170};
    u32 s_idx = 0;
    for (int preset : {3, 1, 4, 2, 0, 3, 0, 1, 0, 3}) {
        const u32 c_sten = stencils[s_idx++], r_sten = c_sten ^ 255;
        const auto out = (s_idx > 7) ? SceneResolution::Size{3840, 2160} : SceneResolution::Size{};
        const auto sz = SceneResolution::ForPreset(preset, out);
        targets.SetSize(sz);
        ClearNative(runtime, scheduler, dispatch, images[color_id], images[depth_id], 0xffff0000, c_sten);
        auto c = targets.Read(color_id, cv);
        assert(ReadbackPixel(scheduler, dispatch, readback, ai.pMappedData, c.image, vk::ImageAspectFlagBits::eColor, c.layout, sz.width, sz.height) == 0xffff0000);
        auto d = targets.Read(depth_id, dv);
        assert((ReadbackPixel(scheduler, dispatch, readback, ai.pMappedData, d.image, vk::ImageAspectFlagBits::eStencil, d.layout, sz.width, sz.height) & 255) == c_sten);
        c = targets.Attachment(color_id, cv); d = targets.Attachment(depth_id, dv);
        auto cmd = scheduler.CommandBuffer();
        const vk::RenderingAttachmentInfo color{.imageView=c.view, .imageLayout=c.layout, .loadOp=vk::AttachmentLoadOp::eClear, .storeOp=vk::AttachmentStoreOp::eStore, .clearValue=vk::ClearValue{.color={.float32=std::array{1.f,0.f,0.f,1.f}}}};
        const vk::RenderingAttachmentInfo depth{.imageView=d.view, .imageLayout=d.layout, .loadOp=vk::AttachmentLoadOp::eClear, .storeOp=vk::AttachmentStoreOp::eStore, .clearValue=vk::ClearValue{.depthStencil={0.75f, r_sten}}};
        cmd.beginRendering({.renderArea={{0,0},{sz.width,sz.height}}, .layerCount=1, .colorAttachmentCount=1, .pColorAttachments=&color, .pDepthAttachment=&depth, .pStencilAttachment=&depth}, dispatch);
        cmd.endRendering(dispatch);
        runtime.Transit(&images[color_id], vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead);
        runtime.FlushBarriers();
        assert(ReadbackPixel(scheduler, dispatch, readback, ai.pMappedData, images[color_id].GetImage(), vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eTransferSrcOptimal, 1920, 1080) == 0xff0000ff);
        ClearNative(runtime, scheduler, dispatch, images[color_id], images[depth_id], 0xff00ff00);
        c = targets.Read(color_id, cv);
        assert(ReadbackPixel(scheduler, dispatch, readback, ai.pMappedData, c.image, vk::ImageAspectFlagBits::eColor, c.layout, sz.width, sz.height) == 0xff00ff00);
        std::printf("Scene targets: %ux%u color/depth/stencil roundtrip PASS\n", sz.width, sz.height);
    }
    TestBloomPyramid(instance, runtime, scheduler, dispatch, views, images, targets, ci, cv, readback, ai.pMappedData);
    scheduler.Finish(); vmaDestroyBuffer(instance.GetAllocator(), readback, alloc);
}
