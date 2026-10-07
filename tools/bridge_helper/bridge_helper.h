/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NO_PROTOTYPES
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

typedef LONG(WINAPI *FdToHandle)(int fd, unsigned int access, unsigned int attributes, HANDLE *handle);

int fail(const char *what, int code);
HANDLE get_wine_handle(int fd);
int init_vulkan_device(const uint8_t *uuid, VkInstance *out_inst, VkPhysicalDevice *out_phys, VkDevice *out_dev);
int create_shared_and_readback(VkPhysicalDevice phys, VkDevice dev, HANDLE handle, VkDeviceSize size,
                              VkBuffer *out_shared, VkDeviceMemory *out_shared_mem,
                              VkBuffer *out_readback, VkDeviceMemory *out_read_mem);
