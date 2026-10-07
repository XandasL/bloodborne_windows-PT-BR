// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "bench_image.h"

inline void InitV07Context(const Gpu& gpu, uint32_t ow, uint32_t oh, int preset,
                          FfxInterface& backend, ffxContext& context, std::vector<unsigned char>& scratch) {
    FfxFsr4V07AssetSet assets{};
    if (!ffxFsr4V07BuildAssetSet(ModelPreset(preset), ow, oh, &assets)) {
        std::fprintf(stderr, "unsupported output size\n"); std::exit(1);
    }
    const char* dir_env = std::getenv("BB_FSR4_DIR");
    const std::string dir = std::string{dir_env && dir_env[0] ? dir_env : "fsr4_shaders"} + "/";
    std::array<std::vector<unsigned char>, FFX_FSR4_VK_PASS_COUNT> code;
    std::vector<unsigned char> initializer, weights;
    const char* opt_env = std::getenv("BB_FSR4_OPT");
    const bool use_opt = !(opt_env && opt_env[0] == '0');
    auto load = [&](const char* name, std::vector<unsigned char>& data) {
        if (use_opt && ReadFile(dir + "opt/" + name, data)) return;
        if (!ReadFile(dir + name, data)) { std::fprintf(stderr, "missing %s%s\n", dir.c_str(), name); std::exit(1); }
    };
    load(assets.pre, code[0]);
    for (uint32_t p = 0; p < FFX_FSR4_MODEL_PASS_COUNT; ++p) load(assets.model[p], code[1 + p]);
    load(assets.post, code[13]); load(assets.rcas, code[14]); load(assets.spdAutoExposure, code[15]);
    load(assets.initializer, initializer); load(assets.prePassWeights, weights);

    std::array<std::string, FFX_FSR4_VK_PASS_COUNT> entries; entries.fill("main");
    for (uint32_t p = 1; p <= FFX_FSR4_MODEL_PASS_COUNT; ++p) entries[p] = "fsr4_model_v07_i8_pass" + std::to_string(p);
    FfxFsr4VkCreateInfo ci{}; ci.device = gpu.device; ci.physicalDevice = gpu.physical;
    for (uint32_t i = 0; i < FFX_FSR4_VK_PASS_COUNT; ++i) ci.shaders[i] = {reinterpret_cast<const uint32_t*>(code[i].data()), code[i].size(), entries[i].c_str()};
    ci.modelInitializer = initializer.data(); ci.modelInitializerSize = initializer.size();
    ci.prePassWeights = weights.data(); ci.prePassWeightsSize = weights.size();
    scratch.assign(ffxFsr4VkGetScratchMemorySize(), 0);
    ci.scratchBuffer = scratch.data(); ci.scratchBufferSize = scratch.size();
    CHECK(ffxFsr4VkCreateContext(&ci, &backend));

    ffxCreateContextDescUpscale desc{}; desc.header.type = FFX_API_CREATE_CONTEXT_DESC_TYPE_UPSCALE;
    desc.flags = FFX_UPSCALE_ENABLE_HIGH_DYNAMIC_RANGE | FFX_UPSCALE_ENABLE_AUTO_EXPOSURE;
    desc.maxRenderSize = {(ow + 7) & ~7u, (oh + 7) & ~7u}; desc.maxUpscaleSize = {(ow + 7) & ~7u, (oh + 7) & ~7u};
    ffxFsr4V07SetBackendInterface(&backend);
    if (ffxFsr4V07CreateContext(&context, &desc.header, nullptr) != FFX_API_RETURN_OK) {
        std::fprintf(stderr, "provider context creation failed\n"); std::exit(1);
    }
    ffxFsr4V07SetBackendInterface(nullptr);
}

inline void DispatchV07Frames(FfxInterface& backend, ffxContext& context, VkCommandBuffer cmd,
                              const Image& color, const Image& depth, const Image& motion, const Image& output,
                              uint32_t rw, uint32_t rh, uint32_t ow, uint32_t oh, int frames,
                              const std::function<void()>& submit) {
    const VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    auto reg_img = [&](const Image& img, VkAccessFlags acc) {
        const FfxFsr4VkExternalImageState s{sizeof(FfxFsr4VkExternalImageState), img.image, img.view,
            VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, acc, VK_IMAGE_LAYOUT_GENERAL, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, acc};
        CHECK(ffxFsr4VkSetExternalImageState(&backend, &s));
    };
    for (int frame = 1; frame <= frames; ++frame) {
        CHECK(ffxFsr4VkBeginFrame(&backend, uint64_t(frame)));
        CHECK(vkBeginCommandBuffer(cmd, &begin));
        reg_img(color, VK_ACCESS_SHADER_READ_BIT); reg_img(depth, VK_ACCESS_SHADER_READ_BIT);
        reg_img(motion, VK_ACCESS_SHADER_READ_BIT); reg_img(output, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
        ffxDispatchDescUpscale d{}; d.header.type = FFX_API_DISPATCH_DESC_TYPE_UPSCALE; d.commandList = cmd;
        d.color = MakeFfxResource(color, FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT, FFX_API_RESOURCE_STATE_COMPUTE_READ);
        d.depth = MakeFfxResource(depth, FFX_SURFACE_FORMAT_R32_FLOAT, FFX_API_RESOURCE_STATE_COMPUTE_READ);
        d.motionVectors = MakeFfxResource(motion, FFX_SURFACE_FORMAT_R16G16_FLOAT, FFX_API_RESOURCE_STATE_COMPUTE_READ);
        d.output = MakeFfxResource(output, FFX_SURFACE_FORMAT_R16G16B16A16_FLOAT, FFX_API_RESOURCE_STATE_UNORDERED_ACCESS);
        d.jitterOffset = {0.25f * float(frame % 4) - 0.375f, 0.125f}; d.motionVectorScale = {1.f, 1.f};
        d.renderSize = {rw, rh}; d.upscaleSize = {ow, oh}; d.enableSharpening = true; d.sharpness = 0.5f;
        d.enableAutoExposure = true; d.frameTimeDelta = 10.f; d.preExposure = 1.f; d.reset = (frame == 1);
        d.cameraNear = 0.05f; d.cameraFar = 3000.f; d.cameraFovAngleVertical = 0.75f; d.viewSpaceToMetersFactor = 1.f;
        if (ffxFsr4V07Dispatch(&context, &d.header) != FFX_API_RETURN_OK) { std::fprintf(stderr, "dispatch failed\n"); std::exit(1); }
        submit();
        CHECK(ffxFsr4VkRetireFrame(&backend, uint64_t(frame)));
    }
}
