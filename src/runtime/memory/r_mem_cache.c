/* SPDX-License-Identifier: MIT
 * PS4 GPU Region Lookup Caching and Clamping.
 * Single responsibility: Fast per-thread region cache and range validation. (~75 LOC)
 */
#include "r_mem_types.h"

extern uint64_t runtime_disabled_optimizations;

typedef struct { uint64_t generation; uintptr_t start, end; int kind; } CachedRegion;
static _Thread_local CachedRegion region_cache[8];
static _Thread_local unsigned region_cache_next;

static const CachedRegion *cached_region(uintptr_t address) {
    uint64_t generation = r_mem_generation();
    if (generation & 1) return NULL;
    for (unsigned i = 0; i < 8; ++i) {
        const CachedRegion *c = &region_cache[i];
        if (c->generation == generation && c->end && c->start <= address && address < c->end) return c;
    }
    return NULL;
}

static void cache_region(size_t i) {
    uint64_t generation = r_mem_generation();
    if (i == g_mem_vma_count || (generation & 1)) return;
    region_cache[region_cache_next++ % 8] = (CachedRegion){generation, g_mem_vmas[i].start, g_mem_vmas[i].end, g_mem_vmas[i].kind};
}

uint64_t runtime_memory_clamp(uintptr_t address, uint64_t size) {
    const CachedRegion *c = (runtime_disabled_optimizations & 1) ? NULL : cached_region(address);
    if (c && c->kind != KIND_RESERVED && size <= c->end - address) return size;
    r_mem_read_lock();
    size_t i = r_mem_vma_index(address);
    if (i < g_mem_vma_count && g_mem_vmas[i].start <= address) cache_region(i);
    uint64_t length = 0;
    for (; i < g_mem_vma_count && length < size; ++i) {
        if (g_mem_vmas[i].start > address + length || g_mem_vmas[i].kind == KIND_RESERVED) break;
        length = g_mem_vmas[i].end - address;
    }
    r_mem_read_unlock();
    return length < size ? length : size;
}

int runtime_memory_region(uintptr_t address, uintptr_t *start, uintptr_t *end, int *mapped) {
    const CachedRegion *c = (runtime_disabled_optimizations & 1) ? NULL : cached_region(address);
    if (c) { *start = c->start; *end = c->end; *mapped = c->kind != KIND_RESERVED; return 1; }
    r_mem_read_lock();
    size_t i = r_mem_vma_index(address);
    int found = i < g_mem_vma_count;
    if (found && g_mem_vmas[i].start <= address) {
        cache_region(i); *start = g_mem_vmas[i].start; *end = g_mem_vmas[i].end; *mapped = g_mem_vmas[i].kind != KIND_RESERVED;
    } else if (found) {
        *start = i ? g_mem_vmas[i-1].end : 0; *end = g_mem_vmas[i].start; *mapped = 0;
    }
    r_mem_read_unlock();
    return found;
}

int runtime_memory_is_mapped(uintptr_t address, uint64_t size) {
    const CachedRegion *c = (runtime_disabled_optimizations & 1) ? NULL : cached_region(address);
    if (c && c->kind != KIND_RESERVED && size <= c->end - address) return 1;
    r_mem_read_lock();
    int res = r_mem_covered(address, address + size, 0);
    r_mem_read_unlock();
    return res;
}
