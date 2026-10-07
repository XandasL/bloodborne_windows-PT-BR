// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <SDL3/SDL.h>
#include "../bbport_overlay.h"
#include "../bbport_settings.h"
#include "imgui.h"

namespace BbOverlay {

extern std::mutex imgui_mutex;
extern bool initialized;
extern std::atomic<bool> menu_open;
extern bool l3_down, r3_down;
extern bool dirty;
extern float base_scale;
extern std::chrono::steady_clock::time_point last_present;
extern float frame_ms_avg;

void SetOpen(bool value);
void DrawMenu();
void DrawFpsCounter(float w, float h);

} // namespace BbOverlay
