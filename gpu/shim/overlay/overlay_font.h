// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#ifdef _WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
alignas(16) static const unsigned char bb_font_ttf[] = {
#embed BB_FONT_PATH
};
#pragma GCC diagnostic pop
static const unsigned char* const bb_font_ttf_end = bb_font_ttf + sizeof(bb_font_ttf);
#else
asm(".section .rodata\n"
    ".balign 16\n"
    ".hidden bb_font_ttf\n"
    ".global bb_font_ttf\n"
    "bb_font_ttf:\n"
    ".incbin \"" BB_FONT_PATH "\"\n"
    ".hidden bb_font_ttf_end\n"
    ".global bb_font_ttf_end\n"
    "bb_font_ttf_end:\n"
    ".previous\n");
extern "C" const unsigned char bb_font_ttf[];
extern "C" const unsigned char bb_font_ttf_end[];
#endif
