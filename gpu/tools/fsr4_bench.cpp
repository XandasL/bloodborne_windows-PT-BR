// SPDX-License-Identifier: GPL-2.0-or-later
#include "bench/bench_inputs.h"
#include "bench/bench_v07.h"
#include "bench/bench_411.h"

int main(int argc, char** argv) {
    uint32_t rw = 2260, rh = 1272, ow = 3840, oh = 2160;
    int preset = 2, frames = 900, pos = 0;
    bool stats = false, fsr411 = false;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--fsr411")) { fsr411 = true; continue; }
        if (!std::strcmp(argv[i], "--stats")) { stats = true; continue; }
        if (pos == 0) std::sscanf(argv[i], "%ux%u", &rw, &rh);
        else if (pos == 1) std::sscanf(argv[i], "%ux%u", &ow, &oh);
        else if (pos == 2) preset = std::atoi(argv[i]);
        else if (pos == 3) frames = std::atoi(argv[i]);
        ++pos;
    }
    setenv("BB_FSR4_PROFILE", "1", 0);
    if (stats) setenv("BB_FSR4_STATS", "1", 1);
    const Gpu gpu = CreateGpu(stats);

    FfxInterface backend{}; ffxContext context{}; std::vector<unsigned char> scratch;
    if (!fsr411) InitV07Context(gpu, ow, oh, preset, backend, context, scratch);

    const VkImageUsageFlags u = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    Image color = CreateImage(gpu, VK_FORMAT_R16G16B16A16_SFLOAT, rw, rh, u, VK_IMAGE_ASPECT_COLOR_BIT);
    Image depth = CreateImage(gpu, VK_FORMAT_D32_SFLOAT, rw, rh, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    Image motion = CreateImage(gpu, VK_FORMAT_R16G16_SFLOAT, rw, rh, u, VK_IMAGE_ASPECT_COLOR_BIT);
    Image output = CreateImage(gpu, VK_FORMAT_R16G16B16A16_SFLOAT, ow, oh, u, VK_IMAGE_ASPECT_COLOR_BIT);

    VkCommandPool pool; VkCommandPoolCreateInfo pci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, gpu.family};
    CHECK(vkCreateCommandPool(gpu.device, &pci, nullptr, &pool));
    VkCommandBuffer cmd; VkCommandBufferAllocateInfo cai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1};
    CHECK(vkAllocateCommandBuffers(gpu.device, &cai, &cmd));
    VkFence fence; VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, 0};
    CHECK(vkCreateFence(gpu.device, &fci, nullptr, &fence));

    auto submit = [&] {
        CHECK(vkEndCommandBuffer(cmd));
        VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, 0, nullptr, nullptr, 1, &cmd, 0, nullptr};
        CHECK(vkQueueSubmit(gpu.queue, 1, &si, fence));
        CHECK(vkWaitForFences(gpu.device, 1, &fence, VK_TRUE, UINT64_MAX));
        CHECK(vkResetFences(gpu.device, 1, &fence));
    };

    PrepareInputs(gpu, cmd, color, depth, motion, output, rw, rh, submit);
    if (fsr411) Run411Benchmark(gpu, cmd, color, depth, motion, output, rw, rh, ow, oh, preset, frames, submit);
    else DispatchV07Frames(backend, context, cmd, color, depth, motion, output, rw, rh, ow, oh, frames, submit);

    CHECK(vkDeviceWaitIdle(gpu.device));
    if (const char* dump = std::getenv("BENCH_DUMP")) {
        const VkDeviceSize bytes = VkDeviceSize(ow) * oh * 8;
        Buffer rb = CreateHostBuffer(gpu, bytes, VK_BUFFER_USAGE_TRANSFER_DST_BIT);
        const VkCommandBufferBeginInfo begin{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        CHECK(vkBeginCommandBuffer(cmd, &begin));
        VkMemoryBarrier mb{VK_STRUCTURE_TYPE_MEMORY_BARRIER, nullptr, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT};
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &mb, 0, nullptr, 0, nullptr);
        VkBufferImageCopy reg{0, 0, 0, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1}, {}, {ow, oh, 1}};
        vkCmdCopyImageToBuffer(cmd, output.image, VK_IMAGE_LAYOUT_GENERAL, rb.buffer, 1, &reg);
        submit();
        if (FILE* f = std::fopen(dump, "wb")) { std::fwrite(rb.data, 1, size_t(bytes), f); std::fclose(f); }
    }
    if (!fsr411) { ffxFsr4V07DestroyContext(&context, nullptr); ffxFsr4VkDestroyContext(reinterpret_cast<FfxFsr4VkContext*>(scratch.data())); }
    return 0;
}
