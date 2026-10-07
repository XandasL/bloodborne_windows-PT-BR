// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "bench_gpu.h"

inline Image CreateImage(const Gpu& gpu, VkFormat format, uint32_t w, uint32_t h,
                         VkImageUsageFlags usage, VkImageAspectFlags aspect) {
    Image img; img.width = w; img.height = h;
    VkImageCreateInfo ci{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0, VK_IMAGE_TYPE_2D, format,
        {w, h, 1}, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, usage, VK_SHARING_MODE_EXCLUSIVE};
    CHECK(vkCreateImage(gpu.device, &ci, nullptr, &img.image));
    VkMemoryRequirements req; vkGetImageMemoryRequirements(gpu.device, img.image, &req);
    VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, req.size,
        MemoryType(gpu, req.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)};
    CHECK(vkAllocateMemory(gpu.device, &ai, nullptr, &img.memory));
    CHECK(vkBindImageMemory(gpu.device, img.image, img.memory, 0));
    VkImageViewCreateInfo vi{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, img.image,
        VK_IMAGE_VIEW_TYPE_2D, format, {}, {aspect, 0, 1, 0, 1}};
    CHECK(vkCreateImageView(gpu.device, &vi, nullptr, &img.view));
    return img;
}

inline FfxApiResource MakeFfxResource(const Image& image, uint32_t format, uint32_t state) {
    FfxApiResource r{};
    r.resource = reinterpret_cast<void*>(image.view);
    r.description.type = FFX_RESOURCE_TYPE_TEXTURE2D;
    r.description.format = format;
    r.description.width = image.width;
    r.description.height = image.height;
    r.description.depth = 1;
    r.description.mipCount = 1;
    r.state = state;
    return r;
}
