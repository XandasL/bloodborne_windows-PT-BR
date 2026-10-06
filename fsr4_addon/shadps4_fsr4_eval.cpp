// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: GPL-2.0-or-later
#include "shadps4_fsr4_state.h"

static FfxApiResource Resource(const ShadFsr4Image& image, uint32_t resource_state) {
    FfxApiResource r{};
    r.resource = reinterpret_cast<void*>(image.view);
    r.description = {FFX_RESOURCE_TYPE_TEXTURE2D, image.ffx_format, image.width,
                     image.height, 1, 1};
    r.state = resource_state;
    return r;
}

static VkResult Register(const ShadFsr4Image& image, bool writable) {
    const VkAccessFlags access =
        VK_ACCESS_SHADER_READ_BIT | (writable ? VK_ACCESS_SHADER_WRITE_BIT : 0u);
    const FfxFsr4VkExternalImageState ext{
        sizeof(FfxFsr4VkExternalImageState), image.image, image.view, image.layout,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, access, image.layout,
        VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, access
    };
    return ffxFsr4VkSetExternalImageState(&g_fsr4_state.backend, &ext);
}

int32_t Fsr4Evaluate(VkCommandBuffer cmd, const ShadFsr4Evaluate* e, uint64_t frame_id) {
    if (!g_fsr4_state.context_ok || !e) return SHADPS4_FSR4_FAILED;
    const VkResult begin = ffxFsr4VkBeginFrame(&g_fsr4_state.backend, frame_id);
    if (begin == VK_NOT_READY) return SHADPS4_FSR4_BUSY;
    if (begin != VK_SUCCESS) {
        Fsr4Fail("FSR 4: no free frame (" + std::to_string(int(begin)) + ")");
        return SHADPS4_FSR4_FAILED;
    }
    if (Register(e->color, false) != VK_SUCCESS || Register(e->depth, false) != VK_SUCCESS ||
        Register(e->motion, false) != VK_SUCCESS || Register(e->output, true) != VK_SUCCESS) {
        Fsr4Fail("FSR 4: image registration failed");
        return SHADPS4_FSR4_ABANDONED;
    }
    ffxDispatchDescUpscale d{};
    d.header.type = FFX_API_DISPATCH_DESC_TYPE_UPSCALE;
    d.commandList = cmd;
    d.color = Resource(e->color, FFX_API_RESOURCE_STATE_COMPUTE_READ);
    d.depth = Resource(e->depth, FFX_API_RESOURCE_STATE_COMPUTE_READ);
    d.motionVectors = Resource(e->motion, FFX_API_RESOURCE_STATE_COMPUTE_READ);
    d.output = Resource(e->output, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
    d.jitterOffset = {e->jitter_x, e->jitter_y};
    d.motionVectorScale = {1.0f, 1.0f};
    d.renderSize = {e->color.width, e->color.height};
    d.upscaleSize = {e->output.width, e->output.height};
    d.enableSharpening = false;
    d.enableAutoExposure = e->hdr == 1;
    d.frameTimeDelta = e->frame_ms;
    d.preExposure = 1.0f;
    d.reset = e->reset != 0;
    d.cameraNear = e->camera_near;
    d.cameraFar = e->camera_far;
    d.cameraFovAngleVertical = e->fov_y;
    d.viewSpaceToMetersFactor = 1.0f;
    if (const auto res = ffxFsr4V07Dispatch(&g_fsr4_state.context, &d.header);
        res != FFX_API_RETURN_OK) {
        Fsr4Fail("FSR 4: dispatch failed (" + std::to_string(res) + ")");
        return SHADPS4_FSR4_ABANDONED;
    }
    return SHADPS4_FSR4_OK;
}
