/* SPDX-License-Identifier: MIT
 * PS4 ImeDialog Virtual Keyboard Provider.
 * Single responsibility: On-screen keyboard input via GPU window host. (~75 LOC)
 */
#include "r_serv_types.h"
#include "gpu/bbgpu.h"

typedef struct {
    int32_t user; uint32_t type; uint64_t languages; uint32_t enter_label, input_method;
    void *filter; uint32_t option, max_length; uint16_t *buffer;
    float x, y; uint32_t halign, valign; const uint16_t *placeholder, *title; int8_t reserved[16];
} ImeParam;

static struct { int running, finished, end_status; uint16_t *buffer; uint32_t max_length; } s_ime;

static void u16_to_u8(const uint16_t *in, size_t limit, char *out, size_t size) {
    size_t n = 0;
    for (size_t i = 0; in && i < limit && in[i] && n + 4 < size; ++i) {
        uint32_t c = in[i];
        if (c < 0x80) out[n++] = (char)c;
        else if (c < 0x800) { out[n++] = (char)(0xC0 | (c >> 6)); out[n++] = (char)(0x80 | (c & 63)); }
        else { out[n++] = (char)(0xE0 | (c >> 12)); out[n++] = (char)(0x80 | ((c >> 6) & 63)); out[n++] = (char)(0x80 | (c & 63)); }
    }
    out[n] = 0;
}

static void u8_to_u16(const char *in, uint16_t *out, uint32_t max) {
    uint32_t n = 0;
    for (const unsigned char *p = (const unsigned char *)in; *p && n + 1 < max;) {
        uint32_t c = *p++;
        if (c >= 0xC0 && *p) c = ((c & 0x1F) << 6) | (*p++ & 0x3F);
        out[n++] = (uint16_t)c;
    }
    out[n < max ? n : max - 1] = 0;
}

static void ime_complete(int status, const char *text) {
    if (!status && s_ime.buffer && s_ime.max_length) u8_to_u16(text, s_ime.buffer, s_ime.max_length);
    s_ime.end_status = status; s_ime.finished = 1; s_ime.running = 0;
    printf("Runtime: ImeDialog %s%s\n", status ? "cancelled" : "text: ", status ? "" : text);
}

ABI int32_t ime_init(const ImeParam *param, const void *extended) {
    (void)extended;
    if (!param || !param->buffer || !param->max_length) return (int32_t)0x80BC0004;
    if (s_ime.running) return (int32_t)0x80BC0003;
    memset(&s_ime, 0, sizeof(s_ime));
    s_ime.buffer = param->buffer; s_ime.max_length = param->max_length; s_ime.running = 1;
    char initial[512] = "", prompt[256] = "";
    u16_to_u8(param->buffer, param->max_length, initial, sizeof(initial));
    u16_to_u8(param->title, 128, prompt, sizeof(prompt));
    const char *preset = getenv("BB_IME_TEXT");
    if (preset) { ime_complete(0, preset); return 0; }
    if (!bbgpu_text_input_begin(initial, prompt[0] ? prompt : "Text")) {
        const char *name = getenv("BB_USER_NAME");
        ime_complete(0, name ? name : initial[0] ? initial : "Hunter");
    }
    return 0;
}

ABI int32_t ime_status(void) {
    if (s_ime.running) {
        char text[512];
        int st = bbgpu_text_input_poll(text, sizeof(text));
        if (st) ime_complete(st == 1 ? 0 : 1, text);
    }
    return s_ime.running ? 1 : s_ime.finished ? 2 : 0;
}

ABI int32_t ime_result(uint32_t *res) {
    if (!s_ime.finished) return (int32_t)0x80BC0101;
    if (res) *res = (uint32_t)s_ime.end_status;
    return 0;
}
ABI int32_t ime_term(void) { memset(&s_ime, 0, sizeof(s_ime)); return 0; }
