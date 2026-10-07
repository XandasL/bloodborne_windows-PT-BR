// SPDX-License-Identifier: GPL-2.0-or-later
#include "overlay_internal.h"

namespace BbOverlay {

static ImGuiKey KeyFromSdl(SDL_Keycode key) {
    switch (key) {
    case SDLK_TAB: return ImGuiKey_Tab;
    case SDLK_LEFT: return ImGuiKey_LeftArrow;
    case SDLK_RIGHT: return ImGuiKey_RightArrow;
    case SDLK_UP: return ImGuiKey_UpArrow;
    case SDLK_DOWN: return ImGuiKey_DownArrow;
    case SDLK_ESCAPE: return ImGuiKey_Escape;
    case SDLK_RETURN: return ImGuiKey_Enter;
    case SDLK_SPACE: return ImGuiKey_Space;
    default: return ImGuiKey_None;
    }
}

bool HandleEvent(const SDL_Event& e) {
    if (!initialized) return false;
    std::scoped_lock lock{imgui_mutex};
    ImGuiIO& io = ImGui::GetIO();

    if (e.type == SDL_EVENT_KEY_DOWN && e.key.key == SDLK_INSERT) {
        SetOpen(!menu_open);
        return true;
    }

    if (e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN || e.type == SDL_EVENT_GAMEPAD_BUTTON_UP) {
        const bool down = e.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN;
        if (e.gbutton.button == SDL_GAMEPAD_BUTTON_LEFT_STICK) l3_down = down;
        if (e.gbutton.button == SDL_GAMEPAD_BUTTON_RIGHT_STICK) r3_down = down;
        if (l3_down && r3_down) {
            SetOpen(!menu_open);
            l3_down = r3_down = false;
            return true;
        }
    }

    if (!menu_open) return false;

    if (e.type == SDL_EVENT_MOUSE_MOTION) {
        io.AddMousePosEvent(e.motion.x, e.motion.y);
    } else if (e.type == SDL_EVENT_MOUSE_BUTTON_DOWN || e.type == SDL_EVENT_MOUSE_BUTTON_UP) {
        int btn = e.button.button == SDL_BUTTON_LEFT ? 0 : e.button.button == SDL_BUTTON_RIGHT ? 1 : 2;
        io.AddMouseButtonEvent(btn, e.type == SDL_EVENT_MOUSE_BUTTON_DOWN);
    } else if (e.type == SDL_EVENT_MOUSE_WHEEL) {
        io.AddMouseWheelEvent(e.wheel.x, e.wheel.y);
    } else if (e.type == SDL_EVENT_KEY_DOWN || e.type == SDL_EVENT_KEY_UP) {
        io.AddKeyEvent(KeyFromSdl(e.key.key), e.type == SDL_EVENT_KEY_DOWN);
    }
    return true;
}

void UpdateTextInput(SDL_Window* window) {
    if (!initialized || !menu_open) return;
    std::scoped_lock lock{imgui_mutex};
    if (ImGui::GetIO().WantTextInput) SDL_StartTextInput(window);
    else SDL_StopTextInput(window);
}

bool CapturesInput() { return menu_open; }

} // namespace BbOverlay
