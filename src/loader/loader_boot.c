/* SPDX-License-Identifier: MIT
 * PS4 Boot Image Parser and Relocation Engine.
 * Single responsibility: Parsing BBPROBE files and preparing the guest image.
 */
#include "loader_types.h"

int loader_load_boot(FILE *f, const char *magic, uint64_t *out_entry, const char *patch_file) {
    uint64_t size = read64(f), entry = read64(f), ns = read64(f), nr = read64(f);
    g_loader_import_count = read64(f);
    uint64_t cap = memcmp(magic, "BBPROBE1", 8) ? read64(f) : 0;
    if (cap & ~UINT64_C(1)) fail("unknown runtime capabilities");
    runtime_start(cap);
    if (!size || size > 512 * 1024 * 1024 || entry >= size || !ns || ns > 64 ||
        nr > 1000000 || g_loader_import_count > 100000)
        fail("boot file limits exceeded");

    int linked = !memcmp(magic, "BBPROBE3", 8) || !memcmp(magic, "BBPROBE4", 8) ||
                 !memcmp(magic, "BBPROBE5", 8);
    int native_libc = linked && (cap & 1);
    uint64_t main_tls[4] = {0}, procparam = 0;
    uint64_t *bindings = calloc(g_loader_import_count ? g_loader_import_count : 1, sizeof(uint64_t));
    uint64_t *binding_kinds = calloc(g_loader_import_count ? g_loader_import_count : 1, sizeof(uint64_t));
    if (!bindings || !binding_kinds) fail("allocation failed");

    uint64_t nb = loader_read_link_metadata(f, magic, size, main_tls, &procparam);
    if (linked) loader_read_bindings(f, nb, bindings, binding_kinds);

    Segment *segments = calloc(ns, sizeof(*segments));
    Reloc *relocs = calloc(nr ? nr : 1, sizeof(*relocs));
    g_loader_names = calloc(g_loader_import_count ? g_loader_import_count : 1, 128);
    if (!segments || !relocs || !g_loader_names) fail("allocation failed");
    for (uint64_t i = 0; i < ns; ++i) {
        segments[i].address = read64(f); segments[i].size = read64(f); segments[i].flags = read64(f);
        if (segments[i].address > size || segments[i].size > size - segments[i].address ||
            segments[i].address % g_page_size || segments[i].flags > 7) fail("bad segment");
    }
    if (fread(g_loader_names, 128, g_loader_import_count, f) != g_loader_import_count)
        fail("truncated import names");
    for (uint64_t i = 0; i < g_loader_import_count; ++i)
        if (!memchr(g_loader_names[i], 0, 128)) fail("unterminated import name");

    for (uint64_t i = 0; i < nr; ++i) {
        relocs[i].target = read64(f); relocs[i].kind = read64(f);
        relocs[i].value = read64(f); relocs[i].addend = read64(f);
        if (!loader_mapped(segments, ns, relocs[i].target, 8) || relocs[i].kind > 2 ||
            (relocs[i].kind && relocs[i].value >= g_loader_import_count) ||
            (relocs[i].kind != 2 && relocs[i].addend) || relocs[i].addend >= g_page_size)
            fail("bad relocation");
    }
    for (uint64_t m = 0; m < g_loader_module_count; ++m)
        if (!loader_mapped(segments, ns, g_loader_modules[m].init, 1) ||
            (g_loader_modules[m].tls_module &&
             !loader_mapped(segments, ns, g_loader_modules[m].tls_address,
                            g_loader_modules[m].tls_memsz)))
            fail("unmapped module metadata");
    if (g_loader_module_count && !loader_mapped(segments, ns, procparam, 64))
        fail("unmapped procparam");

    g_loader_image = (unsigned char*)loader_allocate(round_page(size));
    if (fread(g_loader_image, 1, size, f) != size || fgetc(f) != EOF)
        fail("incorrect image size");
    unsigned char *traps = (unsigned char*)loader_allocate(round_page((g_loader_import_count + 1) * 32));
    setup_import_traps(traps, g_loader_import_count);
    loader_apply_relocations(segments, ns, relocs, nr, traps, bindings, binding_kinds, native_libc);
    if (patch_file) apply_patches(patch_file, segments, ns, relocs, nr);
    loader_protect(traps, round_page((g_loader_import_count + 1) * 32), 5);

    int executable_entry = 0;
    for (uint64_t i = 0; i < ns; ++i) {
        loader_protect(g_loader_image + segments[i].address, round_page(segments[i].size),
                       (unsigned)segments[i].flags);
        if ((segments[i].flags & 1) && entry >= segments[i].address &&
            entry - segments[i].address < segments[i].size) executable_entry = 1;
    }
    if (!executable_entry) fail("entry is not executable");
    if (linked) {
        runtime_set_main_tls(g_loader_image + main_tls[0], main_tls[1], main_tls[2], main_tls[3]);
        runtime_thread_attach_main();
        runtime_set_procparam(g_loader_image + procparam);
    }
    free(bindings); free(binding_kinds); free(segments); free(relocs);
    *out_entry = entry;
    return linked;
}
