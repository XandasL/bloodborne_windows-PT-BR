/* SPDX-License-Identifier: MIT
 * PS4 Pad Exports and Controller Information.
 * Single responsibility: Export table and rumble/status queries. (~65 LOC)
 */
#include "r_pad_types.h"

ABI int32_t pad_init(void); ABI int32_t pad_open(int32_t, int32_t, int32_t, const void*);
ABI int32_t pad_close(int32_t); ABI int32_t pad_read_state(int32_t, PadData*);
ABI int32_t pad_read(int32_t, PadData*, int32_t);

static ABI int32_t pad_ok_handle(int32_t handle) { return handle == PAD_HANDLE ? 0 : ERR_PAD_INVALID_HANDLE; }
static ABI int32_t pad_ok_handle_flag(int32_t handle, uint8_t flag) { (void)flag; return pad_ok_handle(handle); }

static ABI int32_t pad_info(int32_t handle, ControllerInfo *info) {
    if (handle != PAD_HANDLE) return ERR_PAD_INVALID_HANDLE;
    if (!info) return ERR_PAD_INVALID_ARG;
    memset(info, 0, sizeof(*info));
    info->pixel_density = 44.86f; info->resolution_x = 1920; info->resolution_y = 943;
    info->dead_zone_left = info->dead_zone_right = 2;
    info->connection_type = 0; info->connected = 1; info->device_class = 0;
    info->connected_count = g_pad_connected_count ? g_pad_connected_count : 1;
    return 0;
}

static ABI int32_t pad_vibration(int32_t handle, const uint8_t *param) {
    if (handle != PAD_HANDLE) return ERR_PAD_INVALID_HANDLE;
    if (!param) return ERR_PAD_INVALID_ARG;
#if HAVE_SDL3
    r_pad_init_lock();
    bb_platform_mutex_lock(g_pad_lock);
    SDL_Gamepad *g = r_pad_get_gamepad();
    if (g) SDL_RumbleGamepad(g, (uint16_t)(param[0] * 257), (uint16_t)(param[1] * 257), 1000);
    bb_platform_mutex_unlock(g_pad_lock);
#endif
    return 0;
}

static const RuntimeExport exports[] = {
    {"scePadInit", pad_init}, {"scePadOpen", pad_open}, {"scePadClose", pad_close},
    {"scePadReadState", pad_read_state}, {"scePadRead", pad_read},
    {"scePadGetControllerInformation", pad_info}, {"scePadSetVibration", pad_vibration},
    {"scePadResetOrientation", pad_ok_handle},
    {"scePadSetAngularVelocityDeadbandState", pad_ok_handle_flag},
    {"scePadSetTiltCorrectionState", pad_ok_handle_flag},
    {"scePadSetMotionSensorState", pad_ok_handle_flag},
};

uintptr_t runtime_pad_resolve(const char *name) { return RUNTIME_LOOKUP(exports, name); }
void runtime_pad_report(void) {
    printf("Runtime: pad reads=%zu, connected_count=%u\n", g_pad_reads, g_pad_connected_count);
}
