/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

VkDeviceSize caps_largest_local_heap(VkPhysicalDevice device);
int caps_better_device(VkPhysicalDevice candidate, VkPhysicalDevice current);
int caps_has_extension(VkPhysicalDevice device, const char *name);
int caps_live_resolution_suits(VkPhysicalDevice device);
int caps_check_formats(VkPhysicalDevice device, const char *deviceName);
