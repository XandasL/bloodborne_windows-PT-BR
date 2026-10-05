/* SPDX-License-Identifier: MIT
 * PS4 Boot Image Parser and Relocation Engine.
 * Single responsibility: Parsing BBPROBE files and resolving relocations. (~75 LOC)
 */
#include "loader_types.h"

int loader_load_boot(FILE *f, const char *magic, uint64_t *out_entry, const char *patch_file) {
    uint64_t size = read64(f), entry = read64(f), ns = read64(f), nr = read64(f);
    g_loader_import_count = read64(f);
    uint64_t cap = memcmp(magic, "BBPROBE1", 8) ? read64(f) : 0;
    runtime_start(cap);
    if (!size || size > 512*1024*1024 || entry >= size || !ns || ns > 64) fail("boot limits exceeded");
    int multi = !memcmp(magic, "BBPROBE5", 8), linked = !memcmp(magic, "BBPROBE3", 8) || !memcmp(magic, "BBPROBE4", 8) || multi;
    uint64_t main_tls[4] = {0}, procparam = 0, nb = 0;
    if (multi) {
        procparam = read64(f);
        for (int i = 0; i < 4; ++i) main_tls[i] = read64(f);
        g_loader_module_count = read64(f);
        for (uint64_t m = 0; m < g_loader_module_count; ++m) {
            LinkedModule *x = &g_loader_modules[m];
            x->base = read64(f); x->size = read64(f); x->init = read64(f); x->tls_address = read64(f);
            x->tls_memsz = read64(f); x->tls_filesz = read64(f); x->tls_module = read64(f);
        }
        nb = read64(f);
    }
    uint64_t *bindings = (uint64_t*)calloc(g_loader_import_count ? g_loader_import_count : 1, sizeof(uint64_t));
    Segment *segments = (Segment*)calloc(ns, sizeof(Segment));
    Reloc *relocs = (Reloc*)calloc(nr ? nr : 1, sizeof(Reloc));
    g_loader_names = calloc(g_loader_import_count ? g_loader_import_count : 1, 128);
    for (uint64_t i = 0; i < ns; ++i) { segments[i].address = read64(f); segments[i].size = read64(f); segments[i].flags = read64(f); }
    if (fread(g_loader_names, 128, g_loader_import_count, f) != g_loader_import_count) fail("truncated names");
    for (uint64_t i = 0; i < nr; ++i) { relocs[i].target = read64(f); relocs[i].kind = read64(f); relocs[i].value = read64(f); relocs[i].addend = read64(f); }
    g_loader_image = (unsigned char*)loader_allocate(round_page(size));
    if (fread(g_loader_image, 1, size, f) != size) fail("incorrect image size");
    unsigned char *traps = (unsigned char*)loader_allocate(round_page((g_loader_import_count + 1) * 32));
    setup_import_traps(traps, g_loader_import_count);
    for (uint64_t i = 0; i < nr; ++i) {
        uintptr_t val = relocs[i].kind == 1 ? (uintptr_t)(traps + 32 * relocs[i].value) : (uintptr_t)g_loader_image + relocs[i].value;
        if (relocs[i].kind) {
            uintptr_t res = runtime_resolve(g_loader_names[relocs[i].value], relocs[i].kind == 2);
            if (res) val = res + relocs[i].addend;
        }
        memcpy(g_loader_image + relocs[i].target, &val, 8);
    }
    if (patch_file) apply_patches(patch_file, segments, ns, relocs, nr);
    loader_protect(traps, round_page((g_loader_import_count + 1) * 32), 5);
    for (uint64_t i = 0; i < ns; ++i) loader_protect(g_loader_image + segments[i].address, round_page(segments[i].size), (unsigned)segments[i].flags);
    if (linked) {
        runtime_set_main_tls(g_loader_image + main_tls[0], main_tls[1], main_tls[2], main_tls[3]);
        runtime_thread_attach_main();
        runtime_set_procparam(g_loader_image + procparam);
    }
    free(bindings); free(segments); free(relocs);
    *out_entry = entry;
    return linked;
}
