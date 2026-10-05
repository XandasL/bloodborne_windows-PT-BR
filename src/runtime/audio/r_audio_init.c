/* SPDX-License-Identifier: MIT
 * PS4 AudioOut Initialization and Port Creation.
 * Single responsibility: Audio device setup, port allocation, and format setup. (~70 LOC)
 */
#include "r_audio_types.h"

Port g_audio_ports[PORTS];
int g_audio_initialized = 0;
size_t g_audio_buffers_out = 0, g_audio_ports_opened = 0;
static BbMutex *table_lock;
static int sdl_ready = -1;

static void ensure_lock(void) {
    if (!table_lock) {
        BbMutex *m = bb_platform_mutex_create(BB_MUTEX_TYPE_NORMAL);
        if (!__sync_bool_compare_and_swap(&table_lock, NULL, m)) bb_platform_mutex_destroy(m);
    }
}
void r_audio_table_lock(void) { ensure_lock(); bb_platform_mutex_lock(table_lock); }
void r_audio_table_unlock(void) { if (table_lock) bb_platform_mutex_unlock(table_lock); }

int r_audio_sdl_ready(void) {
    if (sdl_ready < 0) {
        const char *mode = getenv("BB_AUDIO");
        sdl_ready = (!mode || strcmp(mode, "none")) && SDL_InitSubSystem(SDL_INIT_AUDIO);
        printf("Runtime: audio backend %s\n", sdl_ready ? SDL_GetCurrentAudioDriver() : "timer (silent)");
    }
    return sdl_ready;
}

int r_audio_port_range(int type, int *first, int *last) {
    switch (type) {
    case 0: *first = 0; *last = 7; return 1;
    case 1: *first = 8; *last = 8; return 1;
    case 2: *first = 9; *last = 12; return 1;
    case 3: *first = 13; *last = 16; return 1;
    case 4: *first = 17; *last = 20; return 1;
    case 126: *first = 21; *last = 22; return 1;
    case 127: *first = 23; *last = 24; return 1;
    default: return 0;
    }
}

Port *r_audio_port_of(int32_t handle, int32_t *error) {
    int id = handle & 0xff;
    if ((handle & 0x3f000000) != 0x20000000 || id >= PORTS) { *error = ERR_INVALID_PORT; return NULL; }
    if (!g_audio_ports[id].used) { *error = ERR_NOT_OPENED; return NULL; }
    return &g_audio_ports[id];
}

ABI int32_t audio_init(void) {
    r_audio_table_lock();
    int32_t r = g_audio_initialized ? ERR_ALREADY_INIT : 0;
    g_audio_initialized = 1;
    r_audio_table_unlock();
    return r;
}
