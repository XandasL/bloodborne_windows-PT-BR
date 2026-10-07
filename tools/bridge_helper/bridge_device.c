/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "bridge_helper.h"

static PFN_vkGetInstanceProcAddr get_instance_proc;
#define LOAD(inst, name) PFN_##name name = (PFN_##name)get_instance_proc(inst, #name)

int fail(const char *what, int code) {
    printf("bridge_helper: %s (%d)\n", what, code);
    fflush(stdout);
    return 1;
}

HANDLE get_wine_handle(int fd) {
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    FdToHandle fd_to_handle = (FdToHandle)GetProcAddress(ntdll, "wine_server_fd_to_handle");
    if (!fd_to_handle) return NULL;
    HANDLE handle = NULL;
    LONG status = fd_to_handle(fd, GENERIC_ALL, 0, &handle);
    return (status == 0) ? handle : NULL;
}

int init_vulkan_device(const uint8_t *uuid, VkInstance *out_inst, VkPhysicalDevice *out_phys, VkDevice *out_dev) {
    HMODULE vulkan = LoadLibraryA("vulkan-1.dll");
    if (!vulkan) return fail("cannot load vulkan-1.dll", (int)GetLastError());
    get_instance_proc = (PFN_vkGetInstanceProcAddr)GetProcAddress(vulkan, "vkGetInstanceProcAddr");
    LOAD(NULL, vkCreateInstance);
    const VkApplicationInfo app = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_3};
    const VkInstanceCreateInfo instance_ci = {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pApplicationInfo = &app};
    VkResult r = vkCreateInstance(&instance_ci, NULL, out_inst);
    if (r) return fail("vkCreateInstance", r);

    LOAD(*out_inst, vkEnumeratePhysicalDevices);
    LOAD(*out_inst, vkGetPhysicalDeviceProperties2);
    LOAD(*out_inst, vkCreateDevice);

    VkPhysicalDevice devices[8]; uint32_t count = 8;
    vkEnumeratePhysicalDevices(*out_inst, &count, devices);
    *out_phys = VK_NULL_HANDLE;
    for (uint32_t i = 0; i < count; ++i) {
        VkPhysicalDeviceIDProperties ids = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ID_PROPERTIES};
        VkPhysicalDeviceProperties2 props = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2, .pNext = &ids};
        vkGetPhysicalDeviceProperties2(devices[i], &props);
        if (!memcmp(ids.deviceUUID, uuid, VK_UUID_SIZE)) {
            *out_phys = devices[i];
            printf("bridge_helper: device %s\n", props.properties.deviceName);
        }
    }
    if (!*out_phys) return fail("no device with the native UUID", (int)count);

    const float priority = 1.0f;
    const VkDeviceQueueCreateInfo queue_ci = {.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = 0, .queueCount = 1, .pQueuePriorities = &priority};
    const char *exts[] = {VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME};
    const VkDeviceCreateInfo device_ci = {.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .queueCreateInfoCount = 1, .pQueueCreateInfos = &queue_ci, .enabledExtensionCount = 1, .ppEnabledExtensionNames = exts};
    r = vkCreateDevice(*out_phys, &device_ci, NULL, out_dev);
    return r ? fail("vkCreateDevice (external_memory_win32)", r) : 0;
}
