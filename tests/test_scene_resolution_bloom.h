// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "test_scene_resolution_setup.h"

inline void TestBloomPyramid(Instance& instance, Runtime& runtime, Scheduler& scheduler,
                             vk::detail::DispatchLoaderDynamic& dispatch,
                             Common::SlotVector<VideoCore::ImageView>& views,
                             Common::SlotVector<VideoCore::Image>& images,
                             SceneTargets& targets, VideoCore::ImageInfo ci, VideoCore::ImageViewInfo cv,
                             VkBuffer readback, void* pMappedData) {
    auto hi = ci; hi.size = {960, 540, 1}; hi.resources = {4, 1};
    const auto half_id = images.insert(instance, runtime, views, hi);
    const auto scene = SceneResolution::ForPreset(4);
    targets.SetSize(scene);
    auto& half = images[half_id];
    assert(targets.Eligible(half) && !targets.EligibleScene(half));
    assert((targets.ProxySize(half, 0) == SceneResolution::Size{320, 180}));
    assert((targets.ProxySize(half, 2) == SceneResolution::Size{80, 45}));
    auto lv = cv; lv.range.base.level = 2;
    const auto proxy = targets.Attachment(half_id, lv);
    {
        auto cmd = scheduler.CommandBuffer();
        const vk::RenderingAttachmentInfo color{.imageView=proxy.view, .imageLayout=proxy.layout,
            .loadOp=vk::AttachmentLoadOp::eClear, .storeOp=vk::AttachmentStoreOp::eStore,
            .clearValue=vk::ClearValue{.color={.float32=std::array{0.f, 1.f, 0.f, 1.f}}}};
        cmd.beginRendering({.renderArea={{0, 0}, {80, 45}}, .layerCount=1, .colorAttachmentCount=1, .pColorAttachments=&color}, dispatch);
        cmd.endRendering(dispatch);
    }
    assert(targets.ProxyCurrent(half, 2, 1) && !targets.ProxyCurrent(half, 0, 4));
    runtime.Transit(&half, vk::ImageLayout::eTransferSrcOptimal, vk::PipelineStageFlagBits2::eTransfer, vk::AccessFlagBits2::eTransferRead);
    runtime.FlushBarriers();
    u32 result = ReadbackPixel(scheduler, dispatch, readback, pMappedData, half.GetImage(),
                              vk::ImageAspectFlagBits::eColor, vk::ImageLayout::eTransferSrcOptimal, 240, 135, 2);
    assert(result == 0xff00ff00);
    std::puts("Scene targets: half-resolution mip level proxy roundtrip PASS");
}
