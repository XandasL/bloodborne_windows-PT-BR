// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "test_taa_common.h"

template<typename F>
inline void RunTaaCases(F&& run) {
    run("reset ignores poisoned history", 1, 0.75f, 0.5f, 0, 1, 0.25f, true);
    run("reset unjitters current color", 1, 0.75f, 0.5f, 0, 1, 0.625f, true, 0.5f, 0, 0.5f);
    run("temporal accumulation", 0, 0.75f, 0.5f, 0, 1, 0.74f);
    run("motion reduces persistence", 0, 0.75f, 0.5f, 4, 1, 0.7075f);
    run("neighborhood clipping", 0, 1, 0.5f, 0, 0.3f, 0.299f);
    run("static disocclusion is clipped", 0, 0.75f, 0.25f, 0, 0.3f, 0.299f);
    run("moving disocclusion is rejected", 0, 0.75f, 0.25f, 2, 1, 0.25f);
    run("out-of-frame motion", 0, 0.75f, 0.5f, 50, 1, 0.25f);
    run("invalid history", 0, 0.75f, 0.5f, 0, 1, 0.25f, true);
    run("far depth retains full precision", 0, 0.75f, 0.99998f, 0, 1, 0.74f, false, 0.99998f);
    run("far unrelated surface rejected", 0, 0.75f, 0.9998f, 2, 1, 0.25f, false, 0.99998f);
    run("camera translation preserves history", 0, 0.75f, 0.75f, 0, 1, 0.74f, false, 0.5f, 0.1f);
    run("bilinear edge keeps matching tap", 0, 0.75f, 0.5f, 0.5f, 1, 0.25f + 0.5f * (0.98f - 0.13f * 0.5f / 8.f) * 0.5f, false, 0.5f, 0, 0, true);
    run("sky retains history", 0, 0.75f, 1.f, 0, 1, 0.74f, false, 1.f);
    run("sky rejects geometry", 0, 0.75f, 0.99998f, 0, 1, 0.25f, false, 1.f);
    run("sloping depth positive jitter", 0, 0.75f, 0.99036f, 0, 1, 0.746f, false, 0.99f, 0, 0.4f, false, 0.0009f);
    run("sloping depth negative jitter", 0, 0.75f, 0.98964f, 0, 1, 0.738f, false, 0.99f, 0, -0.4f, false, 0.0009f);
    run("thin foreground retains history", 0, 0.75f, 0.99f, 0, 1, 0.74f, false, 1.f, 0, 0, false, 0, true);
    run("thin line missed by this frame keeps history", 0, 0.75f, 0.25f, 0, 1, 0.74f, false, 0.5f, 0, 0, false, 0, false, false, true);
    run("tiny motion at large pixel coordinate", 0, 0.75f, 0.5f, -0.00003f, 1, 0.739985f, false, 0.5f, 0, 0, false, 0, false, true);
}
