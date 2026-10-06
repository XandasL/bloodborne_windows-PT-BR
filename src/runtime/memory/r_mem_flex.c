/* SPDX-License-Identifier: MIT
 * PS4 Flexible Memory Mapping and Low-Address Allocations.
 * Single responsibility: Flexible memory lifecycle and low-space mappings. (~75 LOC)
 */
#include "r_mem_types.h"

uint64_t g_mem_flex_bytes = 0;
size_t g_mem_flex_maps = 0;
int32_t r_mem_unmap_locked(uintptr_t start, uint64_t size);

int32_t r_mem_map_flex_locked(void **inout, uint64_t size, int prot, int flags) {
    if (!inout || !size || size % PAGE || (prot & ~0x37)) return INVALID;
    if (g_mem_flex_bytes + size > FLEXIBLE_SIZE || !r_mem_pool()) return NO_MEMORY;
    uint64_t phys = r_mem_flex_alloc(size);
    if (phys == UINT64_MAX) return NO_MEMORY;
    int32_t res = r_mem_place(inout, size, prot, flags, PAGE, KIND_FLEXIBLE, 0, phys);
    if (res) r_mem_flex_free(phys, size); else ++g_mem_flex_maps;
    return res;
}

ABI int32_t map_flexible(void **inout, uint64_t size, int prot, int flags) {
    r_mem_write_lock();
    int32_t res = r_mem_map_flex_locked(inout, size, prot, flags);
    r_mem_write_unlock();
    r_mem_flush_hooks();
    return res;
}

ABI int32_t map_flexible_named(void **inout, uint64_t size, int prot, int flags, const char *name) {
    (void)name; return map_flexible(inout, size, prot, flags);
}

ABI int32_t release_flexible(void *address, uint64_t size) {
    r_mem_write_lock();
    int32_t res = r_mem_unmap_locked((uintptr_t)address, size);
    r_mem_write_unlock();
    r_mem_flush_hooks();
    return res;
}

ABI int32_t reserve_range(void **inout, uint64_t size, int flags, uint64_t alignment) {
    if (!alignment) alignment = PAGE;
    if (!inout || !size || size % PAGE) return INVALID;
    r_mem_write_lock();
    if ((flags & MAP_FIXED_FLAG) && r_mem_overlaps((uintptr_t)*inout, (uintptr_t)*inout + size, 0)) {
        r_mem_write_unlock(); return NO_MEMORY;
    }
    int32_t res = r_mem_place(inout, size, 0, flags, alignment, KIND_RESERVED, 0, 0);
    r_mem_write_unlock();
    return res;
}

#define LOW_MIN UINT64_C(0x0800000000)
static uintptr_t s_low_next = LOW_MIN;

void *runtime_low_map(size_t size, int prot) {
    size = (size + PAGE - 1) & ~(PAGE - 1);
    r_mem_write_lock();
    void *p = NULL;
    while (s_low_next + size <= USER_MIN) {
        p = bb_platform_vm_reserve(s_low_next, size);
        if (p && bb_platform_vm_commit(p, size, prot) == 0) {
            s_low_next += size + PAGE; break;
        }
        if (p) bb_platform_vm_release(p, size);
        p = NULL; s_low_next += size + PAGE;
    }
    r_mem_write_unlock();
    return p;
}
