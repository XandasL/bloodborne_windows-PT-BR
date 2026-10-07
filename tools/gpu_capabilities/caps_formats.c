/* SPDX-License-Identifier: GPL-2.0-or-later */
#include "caps.h"

int caps_check_formats(VkPhysicalDevice device, const char *deviceName) {
    const struct { VkFormat format; const char *name; } formats[] = {
        {VK_FORMAT_R8G8B8A8_UNORM, "RGBA8"},
        {VK_FORMAT_R8G8B8A8_SRGB, "RGBA8 sRGB"},
        {VK_FORMAT_B10G11R11_UFLOAT_PACK32, "B10G11R11"},
        {VK_FORMAT_R16G16B16A16_SFLOAT, "RGBA16F"},
        {VK_FORMAT_D32_SFLOAT_S8_UINT, "D32S8"},
    };
    int supported = 1;
    for (size_t i = 0; i < sizeof(formats) / sizeof(formats[0]); ++i) {
        VkFormatProperties features;
        vkGetPhysicalDeviceFormatProperties(device, formats[i].format, &features);
        const VkFormatFeatureFlags req = VK_FORMAT_FEATURE_BLIT_SRC_BIT | VK_FORMAT_FEATURE_BLIT_DST_BIT;
        if ((features.optimalTilingFeatures & req) != req) {
            fprintf(stderr, "GPU scene scaling: %s lacks blit support for %s\n",
                    deviceName, formats[i].name);
            supported = 0;
        }
    }
    if (supported) {
        fprintf(stderr, "GPU scene scaling: %s supports live presets\n", deviceName);
    }
    return supported;
}
