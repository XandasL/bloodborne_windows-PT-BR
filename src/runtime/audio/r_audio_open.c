/* SPDX-License-Identifier: MIT
 * PS4 AudioOut Port Allocation and Stream Open.
 * Single responsibility: Opening audio port streams and initializing port state. (~55 LOC)
 */
#include "r_audio_types.h"

ABI int32_t audio_open(int32_t user, int32_t type, int32_t index, uint32_t length, uint32_t freq, uint32_t param) {
    (void)user; (void)index;
    if (!g_audio_initialized) return ERR_NOT_INIT;
    if (!length || length > 2048 || (length & 0xff)) return ERR_INVALID_SIZE;
    if (freq != 48000) return ERR_INVALID_FREQ;
    uint32_t format = param & 0xff;
    if (format > 7) return ERR_INVALID_FORMAT;
    int first, last;
    if (!r_audio_port_range(type, &first, &last)) return ERR_INVALID_TYPE;
    static const int channels[8] = {1, 2, 8, 1, 2, 8, 8, 8};
    r_audio_table_lock();
    int id = -1;
    for (int i = first; i <= last; ++i) if (!g_audio_ports[i].used) { id = i; break; }
    if (id < 0) { r_audio_table_unlock(); return ERR_PORT_FULL; }
    Port *p = &g_audio_ports[id];
    memset(p, 0, sizeof(*p));
    p->lock = bb_platform_mutex_create(BB_MUTEX_TYPE_NORMAL);
    p->used = 1; p->type = type; p->channels = channels[format]; p->is_float = format >= 3 && format != 6;
    p->sample_bytes = p->is_float ? 4 : 2; p->frames = (int)length; p->std_layout = format >= 6;
    for (int c = 0; c < 8; ++c) p->volume[c] = VOLUME_0DB;
    if (r_audio_sdl_ready()) {
        SDL_AudioSpec spec = {p->is_float ? SDL_AUDIO_F32 : SDL_AUDIO_S16, p->channels, 48000};
        p->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, NULL, NULL);
        if (p->stream) SDL_ResumeAudioStreamDevice(p->stream);
        else fprintf(stderr, "Runtime: SDL audio stream failed (%s); port %d timer sink\n", SDL_GetError(), id);
    }
    const char *dump = getenv("BB_AUDIO_DUMP");
    if (dump) {
        char path[4096];
        snprintf(path, sizeof(path), "%s.port%d.%dch.%s", dump, id, p->channels, p->is_float ? "f32" : "s16");
        p->dump = fopen(path, "wb");
    }
    ++g_audio_ports_opened;
    r_audio_table_unlock();
    printf("Runtime: audio port %d opened (type %d, %d ch, %s, %u frames)\n", id, type, p->channels, p->is_float ? "float" : "s16", length);
    return (type << 16) | id | 0x20000000;
}
