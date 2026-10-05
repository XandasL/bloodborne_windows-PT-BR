/* SPDX-License-Identifier: MIT
 * PS4 AudioOut Port Teardown and Volume Control.
 * Single responsibility: Closing audio streams and port volume/state queries. (~65 LOC)
 */
#include "r_audio_types.h"

ABI int32_t audio_close(int32_t handle) {
    r_audio_table_lock();
    int32_t error = 0;
    Port *p = r_audio_port_of(handle, &error);
    if (p) {
        bb_platform_mutex_lock(p->lock);
        if (p->stream) SDL_DestroyAudioStream(p->stream);
        if (p->dump) fclose(p->dump);
        p->dump = NULL; p->stream = NULL; p->used = 0;
        bb_platform_mutex_unlock(p->lock);
        bb_platform_mutex_destroy(p->lock);
        p->lock = NULL;
    }
    r_audio_table_unlock();
    return p ? 0 : error;
}

ABI int32_t audio_volume(int32_t handle, int32_t flags, const int32_t *volume) {
    int32_t error = 0;
    r_audio_table_lock();
    Port *p = r_audio_port_of(handle, &error);
    r_audio_table_unlock();
    if (!p) return error;
    if (!volume) return ERR_INVALID_POINTER;
    bb_platform_mutex_lock(p->lock);
    for (int c = 0; c < 8; ++c) if (flags & (1 << c)) {
        if (volume[c] < 0 || volume[c] > VOLUME_0DB) { bb_platform_mutex_unlock(p->lock); return ERR_INVALID_VOLUME; }
        p->volume[c] = volume[c];
    }
    bb_platform_mutex_unlock(p->lock);
    return 0;
}

ABI int32_t audio_state(int32_t handle, PortState *state) {
    int32_t error = 0;
    r_audio_table_lock();
    Port *p = r_audio_port_of(handle, &error);
    r_audio_table_unlock();
    if (!p) return error;
    if (!state) return ERR_INVALID_POINTER;
    memset(state, 0, sizeof(*state));
    switch (p->type) {
    case 2: case 3: state->output = 0x40; state->channel = 1; break;
    case 4: state->output = 0x04; state->channel = 1; state->volume = 127; break;
    default: state->output = 0x01; state->channel = (uint8_t)(p->channels > 2 ? 2 : p->channels); break;
    }
    return 0;
}

ABI int32_t audio_last_time(int32_t handle, uint64_t *time) {
    int32_t error = 0;
    r_audio_table_lock();
    Port *p = r_audio_port_of(handle, &error);
    r_audio_table_unlock();
    if (!p) return error;
    if (!time) return ERR_INVALID_POINTER;
    *time = p->last_output_us; return 0;
}
