/* SPDX-License-Identifier: MIT
 * PS4 AudioOut NID Exports and Status Reporting.
 * Single responsibility: Export table and diagnostic stats. (~45 LOC)
 */
#include "r_audio_types.h"

ABI int32_t audio_init(void);
ABI int32_t audio_open(int32_t user, int32_t type, int32_t index, uint32_t length, uint32_t freq, uint32_t param);
ABI int32_t audio_close(int32_t handle);
ABI int32_t audio_output(int32_t handle, const void *data);
ABI int32_t audio_outputs(const OutputParam *params, uint32_t count);
ABI int32_t audio_volume(int32_t handle, int32_t flags, const int32_t *volume);
ABI int32_t audio_state(int32_t handle, PortState *state);
ABI int32_t audio_last_time(int32_t handle, uint64_t *time);

static const RuntimeExport exports[] = {
    {"sceAudioOutInit", (void*)audio_init}, {"sceAudioOutOpen", (void*)audio_open}, {"sceAudioOutClose", (void*)audio_close},
    {"sceAudioOutOutput", (void*)audio_output}, {"sceAudioOutOutputs", (void*)audio_outputs},
    {"sceAudioOutSetVolume", (void*)audio_volume}, {"sceAudioOutGetPortState", (void*)audio_state},
    {"sceAudioOutGetLastOutputTime", (void*)audio_last_time},
};

uintptr_t runtime_audio_resolve(const char *name) {
    return RUNTIME_LOOKUP(exports, name);
}

void runtime_audio_report(void) {
    printf("Runtime: audio ports opened=%zu, buffers output=%zu\n", g_audio_ports_opened, g_audio_buffers_out);
}
