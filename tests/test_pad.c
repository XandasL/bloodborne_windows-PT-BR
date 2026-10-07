#define _GNU_SOURCE
#include <assert.h>
#include <unistd.h>
#include "../src/runtime_pad.c"
#include "test_pad_virtual.h"

static int capture;
int bbgpu_overlay_captures_input(void) { return capture; }
uintptr_t runtime_lookup(const RuntimeExport *table, size_t count, const char *name) {
    (void)table; (void)count; (void)name; return 0;
}

static void inject(const char *path, const char *tokens) {
    FILE *f = fopen(path, "w"); assert(f);
    fputs(tokens, f); fclose(f); usleep(25000);
}

int main(void) {
    char path[] = "/tmp/bbport-pad-test-XXXXXX";
    int fd = mkstemp(path); assert(fd >= 0); close(fd);
    setenv("BB_PAD_FILE", path, 1);
    setenv("SDL_VIDEODRIVER", "dummy", 1);
    SDL_SetHint(SDL_HINT_GAMECONTROLLER_IGNORE_DEVICES_EXCEPT, "0x1d50/0x6189");
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD));
    assert(pad_init() == 0 && pad_open(1, 0, 0, NULL) == 1);
    PadData data;
    inject(path, "cross l3 touchpad_left");
    assert(pad_read_state(1, &data) == 0 && (data.buttons & (BTN_CROSS | BTN_L3 | BTN_TOUCHPAD)));
    assert(data.touch_count == 1 && data.touches[0].x == 480 && data.touches[0].y == 471);
    inject(path, "touchpad_right");
    assert(pad_read_state(1, &data) == 0 && data.touch_count == 1 && data.touches[0].x == 1440);
    inject(path, "");
    assert(pad_read_state(1, &data) == 0 && data.buttons == 0 && data.touch_count == 0);

    SDL_JoystickID id = attach_test_joystick();
    SDL_Joystick *joystick = SDL_OpenJoystick(id); assert(joystick);
    assert(SDL_SetJoystickVirtualTouchpad(joystick, 0, 0, true, 0.75f, 0.5f, 1.0f));
    assert(SDL_SetJoystickVirtualTouchpad(joystick, 0, 1, true, 0.25f, 1.0f, 1.0f));
    assert(SDL_SetJoystickVirtualButton(joystick, SDL_GAMEPAD_BUTTON_TOUCHPAD, true));
    SDL_UpdateJoysticks(); SDL_UpdateGamepads();
    assert(pad_read_state(1, &data) == 0 && gamepad && data.touch_count == 2);
    assert(data.touches[0].x == 1439 && data.touches[0].y == 471 && data.touches[0].id == 0);
    capture = 1;
    assert(pad_read_state(1, &data) == 0 && data.touch_count == 0 && data.buttons == 0);
    capture = 0;
    assert(SDL_SetJoystickVirtualTouchpad(joystick, 0, 0, false, 0, 0, 0));
    assert(SDL_SetJoystickVirtualTouchpad(joystick, 0, 1, false, 0, 0, 0));
    SDL_UpdateJoysticks(); SDL_UpdateGamepads();
    assert(pad_read_state(1, &data) == 0 && data.touch_count == 1 && data.touches[0].x == 480);
    SDL_CloseJoystick(joystick);
    if (gamepad) SDL_CloseGamepad(gamepad);
    gamepad = NULL;
    assert(SDL_DetachVirtualJoystick(id));
    SDL_Quit(); unlink(path);
    puts("PASS: pad ABI, debug camera chord, left/right clicks, SDL touch coordinates, overlay capture");
}
