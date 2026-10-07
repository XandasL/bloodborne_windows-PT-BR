/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "bridge_helper.h"

int create_shared_and_readback(VkPhysicalDevice phys, VkDevice dev, HANDLE handle, VkDeviceSize size,
                              VkBuffer *out_shared, VkDeviceMemory *out_shared_mem,
                              VkBuffer *out_readback, VkDeviceMemory *out_read_mem) {
    PFN_vkGetDeviceProcAddr gdpa = (PFN_vkGetDeviceProcAddr)GetProcAddress(GetModuleHandleA("vulkan-1.dll"), "vkGetDeviceProcAddr");
#define DLOAD(name) PFN_##name name = (PFN_##name)gdpa(dev, #name)
    DLOAD(vkCreateBuffer); DLOAD(vkGetBufferMemoryRequirements); DLOAD(vkAllocateMemory); DLOAD(vkBindBufferMemory);
    PFN_vkGetPhysicalDeviceMemoryProperties gpm = (PFN_vkGetPhysicalDeviceMemoryProperties)GetProcAddress(GetModuleHandleA("vulkan-1.dll"), "vkGetPhysicalDeviceMemoryProperties");

    const VkExternalMemoryBufferCreateInfo ext_buf = {.sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO, .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT};
    const VkBufferCreateInfo shared_ci = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .pNext = &ext_buf, .size = size, .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    vkCreateBuffer(dev, &shared_ci, NULL, out_shared);
    VkMemoryRequirements req; vkGetBufferMemoryRequirements(dev, *out_shared, &req);
    VkPhysicalDeviceMemoryProperties mem_props; gpm(phys, &mem_props);

    uint32_t type = 0;
    while (!(req.memoryTypeBits & (1u << type))) ++type;
    const VkImportMemoryWin32HandleInfoKHR import = {.sType = VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_KHR, .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT, .handle = handle};
    const VkMemoryAllocateInfo shared_alloc = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .pNext = &import, .allocationSize = req.size, .memoryTypeIndex = type};
    VkResult r = vkAllocateMemory(dev, &shared_alloc, NULL, out_shared_mem);
    if (r) return fail("vkAllocateMemory import", r);
    vkBindBufferMemory(dev, *out_shared, *out_shared_mem, 0);

    const VkBufferCreateInfo read_ci = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT};
    vkCreateBuffer(dev, &read_ci, NULL, out_readback);
    vkGetBufferMemoryRequirements(dev, *out_readback, &req);
    type = 0;
    while (!((req.memoryTypeBits & (1u << type)) && (mem_props.memoryTypes[type].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))) ++type;
    const VkMemoryAllocateInfo read_alloc = {.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = req.size, .memoryTypeIndex = type};
    vkAllocateMemory(dev, &read_alloc, NULL, out_read_mem);
    vkBindBufferMemory(dev, *out_readback, *out_read_mem, 0);
    return 0;
#undef DLOAD
}
