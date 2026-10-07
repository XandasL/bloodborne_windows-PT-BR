// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "bench_image.h"

inline void Run411Benchmark(const Gpu& gpu, VkCommandBuffer cmd, const Image& color,
                           const Image& depth, const Image& motion, const Image& output,
                           uint32_t rw, uint32_t rh, uint32_t ow, uint32_t oh,
                           int preset, int frames, const std::function<void()>& submit) {
    const char* dir411 = std::getenv("BB_FSR411_DIR");
    auto upscaler411 = std::make_unique<Fsr411::Upscaler>(gpu.physical, gpu.device, dir411 && dir411[0] ? dir411 : "fsr4_411");
    VkQueryPool timestamps = VK_NULL_HANDLE;
    VkQueryPoolCreateInfo qci{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, nullptr, 0, VK_QUERY_TYPE_TIMESTAMP, 2};
    CHECK(vkCreateQueryPool(gpu.device, &qci, nullptr, &timestamps));
    VkPhysicalDeviceProperties props; vkGetPhysicalDeviceProperties(gpu.physical, &props);
    const float period_ns = props.limits.timestampPeriod;
    std::printf("FSR 4.1.1 replay %ux%u -> %ux%u, %d frames\n", rw, rh, ow, oh, frames);

    const VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    double gpu_ms = 0.0;
    for (int frame = 1; frame <= frames; ++frame) {
        CHECK(vkBeginCommandBuffer(cmd, &begin));
        vkCmdResetQueryPool(cmd, timestamps, 0, 2);
        vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, timestamps, 0);
        Fsr411::Frame f; f.cmdbuf = cmd;
        f.color = {color.image, color.view, rw, rh}; f.depth = {depth.image, depth.view, rw, rh};
        f.motion = {motion.image, motion.view, rw, rh}; f.output = {output.image, output.view, ow, oh};
        f.render_width = rw; f.render_height = rh; f.ultra_performance = preset >= 4;
        f.jitter[0] = 0.25f * float((frame - 1) % 4) - 0.375f; f.jitter[1] = 0.125f;
        f.sharpen = true; f.sharpness = 0.5f; f.reset = (frame == 1); f.auto_exposure = true;
        if (!upscaler411->Record(f)) { std::fprintf(stderr, "FSR 4.1.1: %s\n", upscaler411->Error().c_str()); std::exit(1); }
        vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, timestamps, 1);
        submit();
        uint64_t ts[2]; CHECK(vkGetQueryPoolResults(gpu.device, timestamps, 0, 2, sizeof(ts), ts, 8, VK_QUERY_RESULT_64_BIT));
        gpu_ms += double(ts[1] - ts[0]) * period_ns * 1e-6;
        if (frame % 300 == 0) {
            std::printf("FSR 4.1.1 replay: %.3f ms/frame (%s)\n", gpu_ms / 300.0, upscaler411->Describe().c_str());
            gpu_ms = 0.0;
        }
    }
}
