/* SPDX-License-Identifier: MIT
 * PS4 AudioOut Runtime Types and Structures.
 * Single responsibility: AudioOut port structures and error definitions. (~70 LOC)
 */
#ifndef R_AUDIO_TYPES_H
#define R_AUDIO_TYPES_H

#include "runtime.h"
#include "platform/bb_common.h"
#include "platform/sync.h"
#include "platform/time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if __has_include(<SDL3/SDL.h>)
#include <SDL3/SDL.h>
#define HAVE_SDL3_AUDIO 1
#else
#define HAVE_SDL3_AUDIO 0
typedef struct SDL_AudioStream SDL_AudioStream;
typedef struct { int format, channels, freq; } SDL_AudioSpec;
#define SDL_INIT_AUDIO 0
#define SDL_AUDIO_S16 0
#define SDL_AUDIO_F32 1
#define SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK 0
static inline int SDL_InitSubSystem(uint32_t f) { (void)f; return 0; }
static inline const char *SDL_GetCurrentAudioDriver(void) { return "none"; }
static inline SDL_AudioStream *SDL_OpenAudioDeviceStream(int d, const SDL_AudioSpec *s, void *cb, void *u) { (void)d; (void)s; (void)cb; (void)u; return NULL; }
static inline int SDL_ResumeAudioStreamDevice(SDL_AudioStream *s) { (void)s; return 0; }
static inline int SDL_PutAudioStreamData(SDL_AudioStream *s, const void *b, int len) { (void)s; (void)b; (void)len; return 0; }
static inline int SDL_GetAudioStreamQueued(SDL_AudioStream *s) { (void)s; return 0; }
static inline void SDL_DestroyAudioStream(SDL_AudioStream *s) { (void)s; }
static inline const char *SDL_GetError(void) { return "SDL3 unavailable"; }
#endif

#define PORTS 25
#define VOLUME_0DB 32768
#define ERR_NOT_OPENED   ((int32_t)0x80260001)
#define ERR_INVALID_PORT ((int32_t)0x80260003)
#define ERR_INVALID_POINTER ((int32_t)0x80260004)
#define ERR_PORT_FULL    ((int32_t)0x80260005)
#define ERR_INVALID_SIZE ((int32_t)0x80260006)
#define ERR_INVALID_FORMAT ((int32_t)0x80260007)
#define ERR_INVALID_FREQ ((int32_t)0x80260008)
#define ERR_INVALID_VOLUME ((int32_t)0x80260009)
#define ERR_INVALID_TYPE ((int32_t)0x8026000A)
#define ERR_ALREADY_INIT ((int32_t)0x8026000E)
#define ERR_NOT_INIT     ((int32_t)0x8026000F)

typedef struct {
    int used, type, channels, is_float, frames, sample_bytes, std_layout;
    int32_t volume[8];
    SDL_AudioStream *stream;
    uint64_t next_deadline_ns;
    int64_t adjust_ns; int window_min, window_count;
    uint64_t last_output_us;
    BbMutex *lock;
    FILE *dump;
    int stats;
    uint64_t stat_start_ns, stat_last_ns, stat_max_gap_ns;
    unsigned stat_starved, stat_buffers; int stat_min_queued;
} Port;

typedef struct { uint16_t output; uint8_t channel, reserved; int16_t volume; uint16_t reroute; uint64_t flag, reserved64[2]; } PortState;
typedef struct { int32_t handle; const void *data; } OutputParam;

void r_audio_table_lock(void); void r_audio_table_unlock(void); int r_audio_sdl_ready(void);
int r_audio_port_range(int type, int *first, int *last); Port *r_audio_port_of(int32_t handle, int32_t *error);

extern Port g_audio_ports[PORTS]; extern int g_audio_initialized;
extern size_t g_audio_buffers_out, g_audio_ports_opened;

#endif /* R_AUDIO_TYPES_H */
