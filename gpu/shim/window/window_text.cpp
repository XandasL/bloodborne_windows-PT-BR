// SPDX-License-Identifier: GPL-2.0-or-later
#include <SDL3/SDL.h>
#include "../sdl_window.h"

namespace Frontend {

void WindowSDL::BeginTextInput(const std::string& initial, const std::string& prompt) {
    std::scoped_lock lock{text_mutex};
    text = initial;
    text_prompt = prompt;
    text_state = 0;
    text_requested = true;
}

int WindowSDL::PollTextInput(std::string& out) {
    std::scoped_lock lock{text_mutex};
    out = text;
    return text_state;
}

void WindowSDL::UpdateTextTitle() {
    const std::string title = text_active
        ? base_title + " \u2014 " + text_prompt + ": " + text + "_  (Enter = OK, Esc = cancel)"
        : base_title;
    SDL_SetWindowTitle(window, title.c_str());
}

} // namespace Frontend
