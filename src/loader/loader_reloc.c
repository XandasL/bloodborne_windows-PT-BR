/* SPDX-License-Identifier: MIT
 * PS4 Loader Relocations and Import Trampolines.
 * Single responsibility: Import trap stubs and unresolved import diagnostics. (~60 LOC)
 */
#include "loader_types.h"

static ABI __attribute__((noreturn)) void unresolved(uint32_t id, uintptr_t argument) {
    if (id >= g_loader_import_count) fail("bad import trap index");
    printf("STOP: first unsupported PS4 import: %s (index %u)\n", g_loader_names[id], id);
    printf("API: %s\n", runtime_import_name(g_loader_names[id]));
    uintptr_t caller = (uintptr_t)__builtin_return_address(0) - (uintptr_t)g_loader_image;
    printf("Caller return offset: 0x%" PRIxPTR "; first argument: 0x%" PRIxPTR "\n", caller, argument);
    for (uint64_t m = 0; m < g_loader_module_count; ++m)
        if (caller >= g_loader_modules[m].base && caller - g_loader_modules[m].base < g_loader_modules[m].size)
            printf("Caller in linked module %" PRIu64 " (%s): +0x%" PRIxPTR "\n",
                   m, m == 0 ? "libc.prx" : "system module", caller - g_loader_modules[m].base);
    runtime_report();
    puts(g_entered_game ? "Original guest entry instructions executed; game initialization incomplete." :
                          "Native libc initialization incomplete; game entry has not run.");
    fflush(NULL);
    exit(20);
}

void setup_import_traps(unsigned char *traps, uint64_t count) {
    for (uint64_t i = 0; i < count; ++i) {
        unsigned char *t = traps + i * 32;
        uint32_t idx = (uint32_t)i;
        uintptr_t handler = (uintptr_t)unresolved;
        t[0] = 0x48; t[1] = 0x89; t[2] = 0xfe; /* mov rsi, rdi: preserve arg0 */
        t[3] = 0xbf; memcpy(t + 4, &idx, 4);   /* mov edi, index */
        t[8] = 0x48; t[9] = 0xb8; memcpy(t + 10, &handler, 8); /* movabs rax, handler */
        t[18] = 0xff; t[19] = 0xe0;             /* jmp rax */
    }
}
