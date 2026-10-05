/* SPDX-License-Identifier: MIT
 * PS4 Direct Memory Allocation and Mapping Handlers.
 * Single responsibility: Physical range management and direct maps. (~75 LOC)
 */
#include "r_mem_types.h"

Block g_mem_blocks[LIMIT];
size_t g_mem_allocations = 0, g_mem_maps = 0;
uint64_t g_mem_live_bytes = 0;

static int valid_alignment(uint64_t a) {
    return a >= PAGE && a <= r_mem_pool_size() && !(a & (a - 1));
}

static uint64_t align_up(uint64_t n, uint64_t a) { return (n + a - 1) & ~(a - 1); }

int direct_range_allocated(uint64_t phys, uint64_t size) {
    for (uint64_t at = phys; at < phys + size;) {
        int found = 0;
        for (int i = 0; i < LIMIT; ++i) {
            if (g_mem_blocks[i].used && at >= g_mem_blocks[i].start &&
                at - g_mem_blocks[i].start < g_mem_blocks[i].size) {
                at = g_mem_blocks[i].start + g_mem_blocks[i].size;
                found = 1; break;
            }
        }
        if (!found) return 0;
    }
    return 1;
}

static int block_type(uint64_t phys) {
    for (int i = 0; i < LIMIT; ++i)
        if (g_mem_blocks[i].used && phys >= g_mem_blocks[i].start &&
            phys - g_mem_blocks[i].start < g_mem_blocks[i].size) return g_mem_blocks[i].type;
    return -1;
}

ABI uint64_t direct_size(void) { return r_mem_pool_size(); }

ABI int32_t direct_allocate(int64_t low, int64_t high, uint64_t size,
                            uint64_t alignment, int type, int64_t *out) {
    if (!alignment) alignment = PAGE;
    if (!out || low < 0 || high < 0 || !size || size % PAGE || !valid_alignment(alignment) || type < 0 || type > 10)
        return INVALID;
    uint64_t pool_sz = r_mem_pool_size();
    uint64_t end = (uint64_t)high < pool_sz ? (uint64_t)high : pool_sz;
    if ((uint64_t)low >= end || size > end - (uint64_t)low) return NO_SPACE;
    r_mem_write_lock();
    if (!r_mem_pool()) { r_mem_write_unlock(); return NO_MEMORY; }
    uint64_t start = align_up((uint64_t)low, alignment);
    int slot = -1;
    for (int i = 0; i < LIMIT; ++i) if (!g_mem_blocks[i].used) { slot = i; break; }
    for (;;) {
        if (slot < 0 || start > end || size > end - start) { r_mem_write_unlock(); return NO_SPACE; }
        int overlap = 0;
        for (int i = 0; i < LIMIT; ++i) {
            Block b = g_mem_blocks[i];
            if (b.used && start < b.start + b.size && b.start < start + size) {
                start = align_up(b.start + b.size, alignment); overlap = 1; break;
            }
        }
        if (!overlap) break;
    }
    g_mem_blocks[slot] = (Block){start, size, type, 1}; *out = (int64_t)start;
    ++g_mem_allocations; g_mem_live_bytes += size;
    r_mem_write_unlock();
    return 0;
}
