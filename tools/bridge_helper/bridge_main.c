/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "bridge_helper.h"

int execute_and_verify(VkDevice dev, VkBuffer shared, VkBuffer readback, VkDeviceMemory read_mem, VkDeviceSize size) {
    PFN_vkGetDeviceProcAddr gdpa = (PFN_vkGetDeviceProcAddr)GetProcAddress(GetModuleHandleA("vulkan-1.dll"), "vkGetDeviceProcAddr");
#define DLOAD(name) PFN_##name name = (PFN_##name)gdpa(dev, #name)
    DLOAD(vkGetDeviceQueue); DLOAD(vkCreateCommandPool); DLOAD(vkAllocateCommandBuffers);
    DLOAD(vkBeginCommandBuffer); DLOAD(vkCmdCopyBuffer); DLOAD(vkCmdPipelineBarrier);
    DLOAD(vkCmdFillBuffer); DLOAD(vkEndCommandBuffer); DLOAD(vkQueueSubmit);
    DLOAD(vkQueueWaitIdle); DLOAD(vkMapMemory);

    VkQueue queue; vkGetDeviceQueue(dev, 0, 0, &queue);
    const VkCommandPoolCreateInfo pool_ci = {.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    VkCommandPool pool; vkCreateCommandPool(dev, &pool_ci, NULL, &pool);
    const VkCommandBufferAllocateInfo cb_ai = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = pool, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = 1};
    VkCommandBuffer cmd; vkAllocateCommandBuffers(dev, &cb_ai, &cmd);
    const VkCommandBufferBeginInfo begin = {.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkBeginCommandBuffer(cmd, &begin);
    const VkBufferCopy region = {.size = size};
    vkCmdCopyBuffer(cmd, shared, readback, 1, &region);
    const VkMemoryBarrier barrier = {.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER, .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT, .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT};
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &barrier, 0, NULL, 0, NULL);
    vkCmdFillBuffer(cmd, shared, 0, size, 0x5EED1234u);
    vkEndCommandBuffer(cmd);
    const VkSubmitInfo submit = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cmd};
    VkResult r = vkQueueSubmit(queue, 1, &submit, VK_NULL_HANDLE);
    if (r) return fail("vkQueueSubmit", r);
    vkQueueWaitIdle(queue);

    uint32_t *words; vkMapMemory(dev, read_mem, 0, size, 0, (void **)&words);
    size_t bad = 0;
    for (size_t i = 0; i < size / 4; ++i) {
        bad += words[i] != 0xB12D6E00u + (uint32_t)(i & 0xFF);
    }
    printf("bridge_helper: native pattern %s (%zu bad words of %zu); wrote 0x5EED1234\n",
           bad ? "WRONG" : "OK", bad, (size_t)(size / 4));
    fflush(stdout);
    return bad ? 2 : 0;
#undef DLOAD
}
