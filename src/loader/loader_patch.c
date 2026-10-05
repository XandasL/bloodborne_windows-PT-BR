/* SPDX-License-Identifier: MIT
 * PS4 Loader BBPATCH2 Runtime Patch Application.
 * Single responsibility: Applying resolution and FPS patches to loaded guest image. (~55 LOC)
 */
#include "loader_types.h"

void apply_patches(const char *path, Segment *segments, uint64_t ns, const Reloc *relocs, uint64_t nr) {
    FILE *f = fopen(path, "rb");
    char magic[8];
    if (!f || fread(magic, 1, 8, f) != 8 || memcmp(magic, "BBPATCH2", 8)) fail("invalid patch file");
    uint64_t base = read64(f), count = read64(f), bytes = 0, rebased = 0;
    unsigned char data[4096];
    for (uint64_t i = 0; i < count; ++i) {
        uint64_t offset = read64(f), length = read64(f);
        if (!length || length > sizeof(data) || !loader_mapped(segments, ns, offset, length) ||
            fread(data, 1, length, f) != length) fail("bad patch entry");
        uint64_t slots[sizeof(data) / 8]; size_t nslots = 0;
        for (uint64_t r = 0; r < nr; ++r) {
            if (!(relocs[r].target < offset + length && offset < relocs[r].target + 8)) continue;
            uint64_t target = relocs[r].target, value;
            if (relocs[r].kind || target < offset || target + 8 > offset + length) fail("patch overlaps a relocation");
            memcpy(&value, data + (target - offset), 8);
            if (value < base || !loader_mapped(segments, ns, value - base, 1)) fail("patch writes a pointer outside image");
            slots[nslots++] = target;
        }
        memcpy(g_loader_image + offset, data, length);
        for (size_t s = 0; s < nslots; ++s) {
            uint64_t val; memcpy(&val, g_loader_image + slots[s], 8);
            val = (uint64_t)(uintptr_t)g_loader_image + (val - base);
            memcpy(g_loader_image + slots[s], &val, 8);
        }
        rebased += nslots; bytes += length;
    }
    if (fgetc(f) != EOF) fail("trailing data in patch file");
    fclose(f);
    printf("Patches: %" PRIu64 " writes, %" PRIu64 " bytes applied, %" PRIu64 " pointers rebased\n", count, bytes, rebased);
}
