/* SPDX-License-Identifier: MIT
 * PS4 AudioOut PCM Output and Stream Pacing.
 * Single responsibility: PCM format conversion, SDL queueing, and cadence pacing. (~72 LOC)
 */
#include "r_audio_types.h"

static int32_t output_port(int32_t handle, const void *data, int pace) {
    int32_t error = 0;
    r_audio_table_lock(); Port *p = r_audio_port_of(handle, &error); r_audio_table_unlock();
    if (!p) return error;
    bb_platform_mutex_lock(p->lock);
    size_t samples = (size_t)p->frames * (size_t)p->channels, bytes = samples * (size_t)p->sample_bytes;
    uint64_t period = (uint64_t)p->frames * 1000000000ULL / 48000ULL;
    if (data) {
        unsigned char converted[2048 * 8 * 4];
        static const int remap[8] = {0, 1, 2, 3, 6, 7, 4, 5};
        for (size_t f = 0; f < (size_t)p->frames; ++f) for (int c = 0; c < p->channels; ++c) {
            int target = p->channels == 8 && !p->std_layout ? remap[c] : c;
            size_t from = f * (size_t)p->channels + (size_t)c, to = f * (size_t)p->channels + (size_t)target;
            float gain = (float)p->volume[c] / VOLUME_0DB;
            if (p->is_float) { float v; memcpy(&v, (const char*)data + from * 4, 4); v *= gain; memcpy(converted + to * 4, &v, 4); }
            else { int16_t v; memcpy(&v, (const char*)data + from * 2, 2); v = (int16_t)((float)v * gain); memcpy(converted + to * 2, &v, 2); }
        }
        if (p->dump) fwrite(converted, 1, bytes, p->dump);
        if (pace) {
            uint64_t now = bb_platform_time_ns();
            if (!p->next_deadline_ns || now > p->next_deadline_ns + 8 * period) p->next_deadline_ns = now;
            else if (now < p->next_deadline_ns) bb_platform_sleep_ns(p->next_deadline_ns - now);
            p->next_deadline_ns += period;
        }
        if (p->stream) {
            int low = 2 * (int)bytes, queued = SDL_GetAudioStreamQueued(p->stream);
            if (queued < (int)bytes) {
                static const unsigned char silence[2048 * 8 * 4];
                SDL_PutAudioStreamData(p->stream, silence, low - queued < (int)bytes ? low - queued : (int)bytes);
                queued = low;
            }
            if (!p->window_count || queued < p->window_min) p->window_min = queued;
            if (++p->window_count == 32) {
                p->adjust_ns = p->window_min > low + 2 * (int)bytes ? (int64_t)(period / 32) : p->window_min < low ? -(int64_t)(period / 32) : 0;
                p->window_count = 0;
            }
            p->next_deadline_ns += (uint64_t)p->adjust_ns;
            SDL_PutAudioStreamData(p->stream, converted, (int)bytes);
        }
        ++g_audio_buffers_out;
    }
    p->last_output_us = bb_platform_time_us();
    bb_platform_mutex_unlock(p->lock);
    return data ? (int32_t)samples : 0;
}

ABI int32_t audio_output(int32_t handle, const void *data) { return output_port(handle, data, 1); }

ABI int32_t audio_outputs(const OutputParam *params, uint32_t count) {
    if (!params) return ERR_INVALID_POINTER;
    if (!count || count > PORTS) return ERR_PORT_FULL;
    int32_t result = 0;
    for (uint32_t i = 0; i < count; ++i) {
        int32_t r = output_port(params[i].handle, params[i].data, i == 0);
        if (r < 0) return r;
        result = r;
    }
    return result;
}
