/* SPDX-License-Identifier: MIT
 * PS4 Loader Linked-Image Metadata.
 * Single responsibility: BBPROBE3/4/5 module and native-binding metadata.
 */
#include "loader_types.h"

uint64_t loader_read_link_metadata(FILE *f, const char *magic, uint64_t size,
                                   uint64_t main_tls[4], uint64_t *procparam) {
    int multi = !memcmp(magic, "BBPROBE5", 8);
    int linked = !memcmp(magic, "BBPROBE3", 8) || !memcmp(magic, "BBPROBE4", 8) || multi;
    if (!linked) return 0;

    uint64_t count = 0;
    if (multi) {
        *procparam = read64(f);
        for (int i = 0; i < 4; ++i) main_tls[i] = read64(f);
        g_loader_module_count = read64(f);
        if (!g_loader_module_count ||
            g_loader_module_count > sizeof(g_loader_modules) / sizeof(*g_loader_modules))
            fail("invalid module count");
        for (uint64_t m = 0; m < g_loader_module_count; ++m) {
            LinkedModule *x = &g_loader_modules[m];
            x->base = read64(f); x->size = read64(f); x->init = read64(f);
            x->tls_address = read64(f); x->tls_memsz = read64(f);
            x->tls_filesz = read64(f); x->tls_module = read64(f);
        }
        count = read64(f);
    } else {
        LinkedModule *x = &g_loader_modules[0];
        g_loader_module_count = 1;
        x->base = read64(f); x->size = read64(f); x->init = read64(f);
        x->tls_address = read64(f); x->tls_memsz = read64(f); x->tls_filesz = read64(f);
        x->tls_module = 2; count = read64(f); *procparam = read64(f);
        if (!memcmp(magic, "BBPROBE4", 8))
            for (int i = 0; i < 4; ++i) main_tls[i] = read64(f);
    }

    if (main_tls[1] > main_tls[2] || main_tls[2] > 1024 * 1024 ||
        main_tls[0] > size || main_tls[1] > size - main_tls[0] ||
        (main_tls[3] & (main_tls[3] - 1)) || main_tls[3] > 4096)
        fail("invalid eboot TLS metadata");
    for (uint64_t m = 0; m < g_loader_module_count; ++m) {
        LinkedModule *x = &g_loader_modules[m];
        if (x->base >= size || !x->size || x->size > size - x->base ||
            x->init < x->base || x->init - x->base >= x->size ||
            (x->tls_module && (x->tls_address >= size ||
             x->tls_memsz > size - x->tls_address || x->tls_memsz > 1024 * 1024 ||
             x->tls_filesz > x->tls_memsz || x->tls_module < 2 || x->tls_module > 7)))
            fail("invalid linked module metadata");
    }
    if (count > g_loader_import_count) fail("invalid binding count");
    return count;
}

void loader_read_bindings(FILE *f, uint64_t count,
                          uint64_t *bindings, uint64_t *binding_kinds) {
    for (uint64_t i = 0; i < count; ++i) {
        uint64_t index = read64(f), address = read64(f), kind = read64(f);
        int inside = 0;
        for (uint64_t m = 0; m < g_loader_module_count; ++m)
            if (address >= g_loader_modules[m].base &&
                address - g_loader_modules[m].base < g_loader_modules[m].size)
                inside = 1;
        if (index >= g_loader_import_count || !inside || (kind != 1 && kind != 2) ||
            bindings[index]) fail("invalid native binding");
        bindings[index] = address;
        binding_kinds[index] = kind;
    }
}
