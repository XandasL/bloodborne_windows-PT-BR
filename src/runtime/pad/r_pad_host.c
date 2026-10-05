/* SPDX-License-Identifier: MIT
 * PS4 Pad Host Sampling via SDL3 or Fallback.
 * Single responsibility: Acquiring host controller axes/buttons and touchpad coords. (~75 LOC)
 */
#include "r_pad_types.h"

SDL_Gamepad *g_current_gamepad = NULL;
uint8_t g_pad_connected_count = 0;
size_t g_pad_reads = 0;
BbMutex *g_pad_lock = NULL;

void r_pad_init_lock(void) {
    if (!g_pad_lock) g_pad_lock = bb_platform_mutex_create(BB_MUTEX_TYPE_NORMAL);
}

SDL_Gamepad *r_pad_get_gamepad(void) {
#if HAVE_SDL3
    static int s_sdl_ready = 0;
    if (!s_sdl_ready) s_sdl_ready = SDL_WasInit(SDL_INIT_GAMEPAD) ? 1 : SDL_InitSubSystem(SDL_INIT_GAMEPAD) ? 1 : -1;
    if (s_sdl_ready < 0) return NULL;
    if (g_current_gamepad && !SDL_GamepadConnected(g_current_gamepad)) {
        SDL_CloseGamepad(g_current_gamepad); g_current_gamepad = NULL;
    }
    if (!g_current_gamepad) {
        int count = 0;
        SDL_JoystickID *ids = SDL_GetGamepads(&count);
        if (ids && count > 0) {
            g_current_gamepad = SDL_OpenGamepad(ids[0]);
            if (g_current_gamepad) {
                ++g_pad_connected_count;
                printf("Runtime: gamepad connected: %s\n", SDL_GetGamepadName(g_current_gamepad));
            }
        }
        SDL_free(ids);
    }
    return g_current_gamepad;
#else
    return NULL;
#endif
}

static uint8_t axis(int16_t v) { int x = (v + 32768) >> 8; return (uint8_t)(x < 0 ? 0 : x > 255 ? 255 : x); }
static uint8_t trigger(int16_t v) { int x = v >> 7; return (uint8_t)(x < 0 ? 0 : x > 255 ? 255 : x); }

void r_pad_sample_host(PadData *d) {
    memset(d, 0, sizeof(*d));
    d->left_x = d->left_y = d->right_x = d->right_y = 128;
    d->orientation[3] = 1.0f;
    d->connected = 1;
    d->connected_count = g_pad_connected_count ? g_pad_connected_count : 1;
    d->timestamp = bb_platform_time_us();
#if HAVE_SDL3
    SDL_Gamepad *g = r_pad_get_gamepad();
    if (bbgpu_overlay_captures_input()) return;
    if (g) {
        static const struct { SDL_GamepadButton sdl; uint32_t ps; } map[] = {
            {SDL_GAMEPAD_BUTTON_SOUTH, BTN_CROSS}, {SDL_GAMEPAD_BUTTON_EAST, BTN_CIRCLE},
            {SDL_GAMEPAD_BUTTON_WEST, BTN_SQUARE}, {SDL_GAMEPAD_BUTTON_NORTH, BTN_TRIANGLE},
            {SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, BTN_L1}, {SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER, BTN_R1},
            {SDL_GAMEPAD_BUTTON_LEFT_STICK, BTN_L3}, {SDL_GAMEPAD_BUTTON_RIGHT_STICK, BTN_R3},
            {SDL_GAMEPAD_BUTTON_START, BTN_OPTIONS}, {SDL_GAMEPAD_BUTTON_BACK, BTN_TOUCHPAD},
            {SDL_GAMEPAD_BUTTON_TOUCHPAD, BTN_TOUCHPAD},
            {SDL_GAMEPAD_BUTTON_DPAD_UP, BTN_UP}, {SDL_GAMEPAD_BUTTON_DPAD_DOWN, BTN_DOWN},
            {SDL_GAMEPAD_BUTTON_DPAD_LEFT, BTN_LEFT}, {SDL_GAMEPAD_BUTTON_DPAD_RIGHT, BTN_RIGHT},
        };
        for (size_t i = 0; i < sizeof(map) / sizeof(*map); ++i) if (SDL_GetGamepadButton(g, map[i].sdl)) d->buttons |= map[i].ps;
        d->left_x = axis(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFTX)); d->left_y = axis(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFTY));
        d->right_x = axis(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_RIGHTX)); d->right_y = axis(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_RIGHTY));
        d->l2 = trigger(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_LEFT_TRIGGER)); d->r2 = trigger(SDL_GetGamepadAxis(g, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));
        if (d->l2 > 30) d->buttons |= BTN_L2;
        if (d->r2 > 30) d->buttons |= BTN_R2;
    }
#endif
}
