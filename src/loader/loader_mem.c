/* SPDX-License-Identifier: MIT
 * PS4 Loader Memory Allocation and Page Protection.
 * Single responsibility: Low-space memory allocation and section permissions. (~65 LOC)
 */
#include "loader_types.h"

unsigned char *g_loader_image = NULL;
char (*g_loader_names)[128] = NULL;
uint64_t g_loader_import_count = 0;
LinkedModule g_loader_modules[16];
uint64_t g_loader_module_count = 0;
int g_entered_game = 0;
size_t g_page_size = 16384;

void fail(const char *message) { fprintf(stderr, "ERROR: %s\n", message); exit(1); }

uint64_t read64(FILE *f) {
    unsigned char b[8];
    if (fread(b, 1, 8, f) != 8) fail("truncated boot file");
    uint64_t n = 0;
    for (int i = 7; i >= 0; --i) n = (n << 8) | b[i];
    return n;
}

size_t round_page(size_t size) { return (size + g_page_size - 1) & ~(g_page_size - 1); }

void *loader_allocate(size_t size) {
    void *low = runtime_low_map(size, BB_PROT_READ | BB_PROT_WRITE);
    if (low) return low;
    void *p = bb_platform_vm_reserve(0, size);
    if (!p || bb_platform_vm_commit(p, size, BB_PROT_READ | BB_PROT_WRITE) != 0) fail("loader_allocate failed");
    return p;
}

void loader_protect(void *p, size_t size, unsigned flags) {
    int prot = ((flags & 4) ? BB_PROT_READ : 0) | ((flags & 2) ? BB_PROT_WRITE : 0) | ((flags & 1) ? BB_PROT_EXEC : 0);
    if (bb_platform_vm_protect(p, size, prot) != 0) fail("loader_protect failed");
}

int loader_mapped(Segment *segments, uint64_t count, uint64_t address, uint64_t bytes) {
    for (uint64_t i = 0; i < count; ++i)
        if (address >= segments[i].address && bytes <= segments[i].size &&
            address - segments[i].address <= segments[i].size - bytes) return 1;
    return 0;
}
