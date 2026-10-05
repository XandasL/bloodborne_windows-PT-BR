/* SPDX-License-Identifier: MIT
 * PS4 Pad Port Lifecycle and Data Reading.
 * Single responsibility: Opening pad ports and delivering sampled state. (~70 LOC)
 */
#include "r_pad_types.h"

static int s_initialized = 0, s_opened = 0;

void r_pad_sample_host(PadData *d);
void r_pad_apply_inject(PadData *d);

static void sample(PadData *d) {
    r_pad_sample_host(d);
    r_pad_apply_inject(d);
}

ABI int32_t pad_init(void) {
    r_pad_init_lock();
    bb_platform_mutex_lock(g_pad_lock);
    s_initialized = 1;
    bb_platform_mutex_unlock(g_pad_lock);
    return 0;
}

ABI int32_t pad_open(int32_t user, int32_t type, int32_t index, const void *param) {
    (void)param;
    if (!s_initialized) return ERR_PAD_NOT_INITIALIZED;
    if (user != 1 || (type != 0 && type != 2) || index) return ERR_PAD_INVALID_ARG;
    r_pad_init_lock();
    bb_platform_mutex_lock(g_pad_lock);
    int already = s_opened; s_opened = 1;
    bb_platform_mutex_unlock(g_pad_lock);
    if (already) return ERR_PAD_ALREADY_OPENED;
    puts("Runtime: pad opened for user 1 (gamepad or keyboard)");
    return PAD_HANDLE;
}

ABI int32_t pad_close(int32_t handle) {
    if (handle != PAD_HANDLE || !s_opened) return ERR_PAD_INVALID_HANDLE;
    s_opened = 0; return 0;
}

ABI int32_t pad_read_state(int32_t handle, PadData *data) {
    if (handle != PAD_HANDLE || !s_opened) return ERR_PAD_INVALID_HANDLE;
    if (!data) return ERR_PAD_INVALID_ARG;
    r_pad_init_lock();
    bb_platform_mutex_lock(g_pad_lock);
    sample(data); ++g_pad_reads;
    bb_platform_mutex_unlock(g_pad_lock);
    return 0;
}

ABI int32_t pad_read(int32_t handle, PadData *data, int32_t count) {
    if (handle != PAD_HANDLE || !s_opened) return ERR_PAD_INVALID_HANDLE;
    if (!data || count < 1 || count > 64) return ERR_PAD_INVALID_ARG;
    pad_read_state(handle, data);
    return 1;
}
