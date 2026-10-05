/* SPDX-License-Identifier: MIT
 * PS4 Pad Script Injection and Record/Replay Engine.
 * Single responsibility: Test automation input playback from script files. (~75 LOC)
 */
#include "r_pad_types.h"
#include "platform/fs.h"

static struct { uint32_t buttons; int stick[4]; int touch_side; } s_injected = {0, {-1,-1,-1,-1}, -1};
static int s_replay_armed = 0;
static uint64_t s_replay_start = 0;
typedef struct { uint32_t ms, buttons; uint8_t axes[4], l2, r2; } PadSample;
static PadSample *s_replay = NULL;
static size_t s_replay_count = 0, s_replay_next = 0;

void r_pad_read_inject(void) {
    const char *path = getenv("BB_PAD_FILE");
    if (!path || !*path) return;
    FILE *f = fopen(path, "r");
    if (!f) return;
    static const struct { const char *name; uint32_t ps; } names[] = {
        {"cross", BTN_CROSS}, {"circle", BTN_CIRCLE}, {"square", BTN_SQUARE}, {"triangle", BTN_TRIANGLE},
        {"l1", BTN_L1}, {"r1", BTN_R1}, {"l2", BTN_L2}, {"r2", BTN_R2}, {"l3", BTN_L3}, {"r3", BTN_R3},
        {"options", BTN_OPTIONS}, {"touchpad", BTN_TOUCHPAD},
        {"up", BTN_UP}, {"down", BTN_DOWN}, {"left", BTN_LEFT}, {"right", BTN_RIGHT},
    };
    static const char *sticks[] = {"lx=", "ly=", "rx=", "ry="};
    s_injected.buttons = 0; s_injected.touch_side = -1;
    for (int i = 0; i < 4; ++i) s_injected.stick[i] = -1;
    char token[64];
    while (fscanf(f, "%63s", token) == 1) {
        if (!strcmp(token, "replay") && s_replay_armed != 1) { s_replay_armed = 1; s_replay_start = 0; }
        if (!strcmp(token, "touchpad_left") || !strcmp(token, "touchpad_right")) {
            s_injected.buttons |= BTN_TOUCHPAD;
            s_injected.touch_side = !strcmp(token, "touchpad_right");
        }
        for (size_t i = 0; i < sizeof(names)/sizeof(*names); ++i) if (!strcmp(token, names[i].name)) s_injected.buttons |= names[i].ps;
        for (int i = 0; i < 4; ++i) if (!strncmp(token, sticks[i], 3)) { int v = atoi(token + 3); s_injected.stick[i] = v < 0 ? 0 : v > 255 ? 255 : v; }
    }
    fclose(f);
}

void r_pad_apply_inject(PadData *d) {
    r_pad_read_inject();
    d->buttons |= s_injected.buttons;
    if (s_injected.buttons & BTN_L2) d->l2 = 255;
    if (s_injected.buttons & BTN_R2) d->r2 = 255;
    uint8_t *axes[4] = {&d->left_x, &d->left_y, &d->right_x, &d->right_y};
    for (int i = 0; i < 4; ++i) if (s_injected.stick[i] >= 0) *axes[i] = (uint8_t)s_injected.stick[i];
}
