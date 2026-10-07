// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "bench_types.h"

inline uint32_t MemoryType(const Gpu& gpu, uint32_t bits, VkMemoryPropertyFlags flags) {
    for (uint32_t i = 0; i < gpu.memory.memoryTypeCount; ++i) {
        if ((bits & (1u << i)) && (gpu.memory.memoryTypes[i].propertyFlags & flags) == flags) return i;
    }
    std::fprintf(stderr, "no memory type\n"); std::exit(1);
}

inline Gpu CreateGpu(bool stats) {
    Gpu gpu;
    VkApplicationInfo app{VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, "fsr4-bench", 0, nullptr, 0, VK_API_VERSION_1_3};
    VkInstanceCreateInfo ici{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &app};
    CHECK(vkCreateInstance(&ici, nullptr, &gpu.instance));
    uint32_t count = 0; CHECK(vkEnumeratePhysicalDevices(gpu.instance, &count, nullptr));
    std::vector<VkPhysicalDevice> devices(count);
    CHECK(vkEnumeratePhysicalDevices(gpu.instance, &count, devices.data()));
    gpu.physical = devices.at(0);
    for (const auto d : devices) {
        VkPhysicalDeviceProperties p; vkGetPhysicalDeviceProperties(d, &p);
        if (p.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) { gpu.physical = d; break; }
    }
    VkPhysicalDeviceProperties props; vkGetPhysicalDeviceProperties(gpu.physical, &props);
    std::printf("GPU: %s\n", props.deviceName);
    vkGetPhysicalDeviceMemoryProperties(gpu.physical, &gpu.memory);
    uint32_t families = 0; vkGetPhysicalDeviceQueueFamilyProperties(gpu.physical, &families, nullptr);
    std::vector<VkQueueFamilyProperties> family_props(families);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu.physical, &families, family_props.data());
    while (gpu.family < families && !(family_props[gpu.family].queueFlags & VK_QUEUE_COMPUTE_BIT)) ++gpu.family;

    VkPhysicalDevicePipelineExecutablePropertiesFeaturesKHR exec{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_EXECUTABLE_PROPERTIES_FEATURES_KHR};
    VkPhysicalDeviceComputeShaderDerivativesFeaturesKHR deriv{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMPUTE_SHADER_DERIVATIVES_FEATURES_KHR};
    VkPhysicalDeviceCooperativeMatrixFeaturesKHR coop{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_KHR};
    VkPhysicalDeviceShaderMixedFloatDotProductFeaturesVALVE dot{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_MIXED_FLOAT_DOT_PRODUCT_FEATURES_VALVE};
    VkPhysicalDeviceVulkan13Features f13{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES};
    VkPhysicalDeviceVulkan12Features f12{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    VkPhysicalDeviceVulkan11Features f11{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES};
    VkPhysicalDeviceFeatures2 feat{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &f11};
    f11.pNext = &f12; f12.pNext = &f13; f13.pNext = &deriv; deriv.pNext = &coop; coop.pNext = &dot;
    if (stats) dot.pNext = &exec;
    vkGetPhysicalDeviceFeatures2(gpu.physical, &feat);
    feat.features.robustBufferAccess = VK_FALSE; f13.robustImageAccess = VK_FALSE;
    std::vector<const char*> ext{VK_KHR_COMPUTE_SHADER_DERIVATIVES_EXTENSION_NAME, VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME};
    if (dot.shaderMixedFloatDotProductFloat16AccFloat32) ext.push_back(VK_VALVE_SHADER_MIXED_FLOAT_DOT_PRODUCT_EXTENSION_NAME);
    if (coop.cooperativeMatrix) ext.push_back(VK_KHR_COOPERATIVE_MATRIX_EXTENSION_NAME);
    if (stats) ext.push_back(VK_KHR_PIPELINE_EXECUTABLE_PROPERTIES_EXTENSION_NAME);

    const float priority = 1.0f;
    VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, gpu.family, 1, &priority};
    VkDeviceCreateInfo dci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, &feat, 0, 1, &qci, 0, nullptr, uint32_t(ext.size()), ext.data()};
    CHECK(vkCreateDevice(gpu.physical, &dci, nullptr, &gpu.device));
    vkGetDeviceQueue(gpu.device, gpu.family, 0, &gpu.queue);
    return gpu;
}

inline Buffer CreateHostBuffer(const Gpu& gpu, VkDeviceSize size, VkBufferUsageFlags usage) {
    Buffer b;
    VkBufferCreateInfo ci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, size, usage, VK_SHARING_MODE_EXCLUSIVE};
    CHECK(vkCreateBuffer(gpu.device, &ci, nullptr, &b.buffer));
    VkMemoryRequirements req; vkGetBufferMemoryRequirements(gpu.device, b.buffer, &req);
    VkMemoryAllocateInfo ai{VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, req.size,
        MemoryType(gpu, req.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)};
    CHECK(vkAllocateMemory(gpu.device, &ai, nullptr, &b.memory));
    CHECK(vkBindBufferMemory(gpu.device, b.buffer, b.memory, 0));
    CHECK(vkMapMemory(gpu.device, b.memory, 0, VK_WHOLE_SIZE, 0, &b.data));
    return b;
}
