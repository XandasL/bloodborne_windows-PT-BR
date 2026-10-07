/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "bridge_helper.h"

int execute_and_verify(VkDevice dev, VkBuffer shared, VkBuffer readback, VkDeviceMemory read_mem, VkDeviceSize size);

int main(int argc, char **argv) {
    if (argc < 4) return fail("usage: bridge_helper <fd> <size> <uuid>", 0);
    const int fd = atoi(argv[1]);
    const VkDeviceSize size = strtoull(argv[2], NULL, 10);
    uint8_t uuid[VK_UUID_SIZE];
    for (int i = 0; i < VK_UUID_SIZE; ++i) {
        unsigned value = 0;
        sscanf(argv[3] + 2 * i, "%2x", &value);
        uuid[i] = (uint8_t)value;
    }
    HANDLE handle = get_wine_handle(fd);
    if (!handle) return fail("wine_server_fd_to_handle failed", 0);

    VkInstance instance; VkPhysicalDevice physical; VkDevice device;
    if (init_vulkan_device(uuid, &instance, &physical, &device)) return 1;

    VkBuffer shared, readback; VkDeviceMemory shared_mem, read_mem;
    if (create_shared_and_readback(physical, device, handle, size, &shared, &shared_mem, &readback, &read_mem)) return 1;

    return execute_and_verify(device, shared, readback, read_mem, size);
}
