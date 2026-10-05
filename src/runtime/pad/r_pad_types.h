/* SPDX-License-Identifier: MIT
 * PS4 Pad Subsystem Types and Button Constants.
 * Single responsibility: DualShock 4 data layouts and button bitmasks. (~65 LOC)
 */
#ifndef R_PAD_TYPES_H
#define R_PAD_TYPES_H

#include "runtime.h"
#include "gpu/bbgpu.h"
#include "platform/time.h"
#include "platform/sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#if __has_include(<SDL3/SDL.h>)
#include <SDL3/SDL.h>
#define HAVE_SDL3 1
#else
#define HAVE_SDL3 0
typedef struct SDL_Gamepad SDL_Gamepad;
#endif

#define ERR_PAD_INVALID_ARG ((int32_t)0x80920001)
#define ERR_PAD_INVALID_HANDLE ((int32_t)0x80920003)
#define ERR_PAD_ALREADY_OPENED ((int32_t)0x80920004)
#define ERR_PAD_NOT_INITIALIZED ((int32_t)0x80920005)
#define PAD_HANDLE 1

enum {
    BTN_L3=0x2, BTN_R3=0x4, BTN_OPTIONS=0x8, BTN_UP=0x10, BTN_RIGHT=0x20, BTN_DOWN=0x40, BTN_LEFT=0x80,
    BTN_L2=0x100, BTN_R2=0x200, BTN_L1=0x400, BTN_R1=0x800, BTN_TRIANGLE=0x1000, BTN_CIRCLE=0x2000,
    BTN_CROSS=0x4000, BTN_SQUARE=0x8000, BTN_TOUCHPAD=0x100000,
};

typedef struct { uint16_t x, y; uint8_t id, reserve[3]; } PadTouch;
typedef struct {
    uint32_t buttons;
    uint8_t left_x, left_y, right_x, right_y;
    uint8_t l2, r2, analog_padding[2];
    float orientation[4], acceleration[3], angular_velocity[3];
    uint8_t touch_count, touch_reserve[3];
    uint32_t touch_held_time;
    PadTouch touches[2];
    uint8_t connected, pad0[3];
    uint64_t timestamp;
    uint8_t extension[16];
    uint8_t connected_count, reserve[2], unique_length, unique[12];
} PadData;

typedef struct {
    float pixel_density; uint16_t resolution_x, resolution_y;
    uint8_t dead_zone_left, dead_zone_right, connection_type, connected_count;
    uint8_t connected, pad[3];
    int32_t device_class;
    uint8_t reserve[8];
} ControllerInfo;

extern SDL_Gamepad *g_current_gamepad;
extern uint8_t g_pad_connected_count;
extern size_t g_pad_reads;
extern BbMutex *g_pad_lock;

void r_pad_init_lock(void);
SDL_Gamepad *r_pad_get_gamepad(void);

#endif /* R_PAD_TYPES_H */
