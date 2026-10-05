/* SPDX-License-Identifier: MIT
 * PS4 Direct Memory Pool and Flexible Memory Allocator.
 * Single responsibility: Physical memory pool and flexible bitmap. (~65 LOC)
 */
#include "r_mem_types.h"

static BbDirectPool *g_direct_pool;
static uint64_t flex_bitmap[FLEX_SPAN / PAGE / 64];

uint64_t r_mem_pool_size(void) {
    static uint64_t size;
    if (!size) {
        const char *env = getenv("BB_DMEM_MB");
        uint64_t mb = env ? strtoull(env, NULL, 10) : 0;
        if (mb < 5056 || mb > 16384) mb = 5056;
        size = mb * 1024 * 1024;
    }
    return size;
}

BbDirectPool* r_mem_pool(void) {
    if (!g_direct_pool) {
        g_direct_pool = bb_platform_direct_create(r_mem_pool_size() + FLEX_SPAN);
    }
    return g_direct_pool;
}

uint8_t* r_mem_backing(void) {
    BbDirectPool *p = r_mem_pool();
    return p ? (uint8_t*)bb_platform_direct_backing_base(p) : NULL;
}

static int flex_test(uint64_t page) {
    return (int)((flex_bitmap[page / 64] >> (page % 64)) & 1);
}

static void flex_set(uint64_t first, uint64_t count, int used) {
    for (uint64_t p = first; p < first + count; ++p) {
        if (used) flex_bitmap[p / 64] |= UINT64_C(1) << (p % 64);
        else flex_bitmap[p / 64] &= ~(UINT64_C(1) << (p % 64));
    }
}

uint64_t r_mem_flex_alloc(uint64_t size) {
    uint64_t pages = size / PAGE, total = FLEX_SPAN / PAGE, run = 0;
    for (uint64_t p = 0; p < total; ++p) {
        run = flex_test(p) ? 0 : run + 1;
        if (run == pages) {
            flex_set(p + 1 - pages, pages, 1);
            return r_mem_pool_size() + (p + 1 - pages) * PAGE;
        }
    }
    return UINT64_MAX;
}

void r_mem_flex_free(uint64_t phys, uint64_t size) {
    flex_set((phys - r_mem_pool_size()) / PAGE, size / PAGE, 0);
    bb_platform_direct_punch_hole(r_mem_pool(), phys, size);
}
