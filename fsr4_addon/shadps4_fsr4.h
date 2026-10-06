// SPDX-FileCopyrightText: Copyright 2026 IFreemz
// SPDX-License-Identifier: MIT
#pragma once

#include <stdint.h>
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SHADPS4_FSR4_ABI 2

typedef void (*ShadFsr4LogFn)(int warning, const char* message);

typedef struct ShadFsr4Image {
    VkImage image;
    VkImageView view;
    VkImageLayout layout;
    uint32_t width;
    uint32_t height;
    uint32_t ffx_format;
} ShadFsr4Image;

typedef struct ShadFsr4Context {
    uint32_t render_width;
    uint32_t render_height;
    uint32_t output_width;
    uint32_t output_height;
} ShadFsr4Context;

typedef struct ShadFsr4Evaluate {
    ShadFsr4Image color;
    ShadFsr4Image depth;
    ShadFsr4Image motion;
    ShadFsr4Image output;
    float jitter_x, jitter_y;
    float frame_ms;
    float camera_near, camera_far, fov_y;
    int32_t reset;
    int32_t hdr;
} ShadFsr4Evaluate;

enum {
    SHADPS4_FSR4_FAILED = 0,
    SHADPS4_FSR4_OK = 1,
    SHADPS4_FSR4_BUSY = 2,
    SHADPS4_FSR4_ABANDONED = 3
};

typedef struct ShadFsr4Api {
    uint32_t abi;
    int32_t (*Configure)(const wchar_t* directory, ShadFsr4LogFn log);
    int32_t (*Initialize)(VkPhysicalDevice physical, VkDevice device);
    int32_t (*HasContext)(const ShadFsr4Context* context);
    int32_t (*CreateContext)(const ShadFsr4Context* context);
    int32_t (*Evaluate)(VkCommandBuffer command, const ShadFsr4Evaluate* evaluate,
                        uint64_t frame_id);
    void (*Retire)(uint64_t completed_frame_id);
    void (*ReleaseContext)(void);
    const char* (*Problem)(void);
} ShadFsr4Api;

typedef const ShadFsr4Api* (*ShadFsr4GetApiFn)(void);

#ifdef __cplusplus
}
#endif
