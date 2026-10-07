#pragma once
#include <SDL3/SDL.h>
#include <assert.h>

static inline SDL_JoystickID attach_test_joystick(void) {
    SDL_VirtualJoystickTouchpadDesc touch = {.nfingers = 2};
    SDL_VirtualJoystickDesc desc;
    SDL_INIT_INTERFACE(&desc);
    desc.type = SDL_JOYSTICK_TYPE_GAMEPAD;
    desc.naxes = SDL_GAMEPAD_AXIS_COUNT;
    desc.nbuttons = SDL_GAMEPAD_BUTTON_COUNT;
    desc.button_mask = (1u << SDL_GAMEPAD_BUTTON_COUNT) - 1;
    desc.axis_mask = (1u << SDL_GAMEPAD_AXIS_COUNT) - 1;
    desc.name = "bbport test controller";
    desc.vendor_id = 0x1d50;
    desc.product_id = 0x6189;
    desc.ntouchpads = 1;
    desc.touchpads = &touch;
    SDL_JoystickID id = SDL_AttachVirtualJoystick(&desc);
    assert(id != 0);
    return id;
}
