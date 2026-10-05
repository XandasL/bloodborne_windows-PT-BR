/* SPDX-License-Identifier: MIT
 * PS4 Virtual Memory Range Placement and Free-Space Search.
 * Single responsibility: User-space layout carving and placement. (~75 LOC)
 */
#include "r_mem_types.h"

int r_mem_covered(uintptr_t start, uintptr_t end, int allow_reserved) {
    size_t i = r_mem_vma_index(start); uintptr_t at = start;
    for (; at < end; ++i) {
        if (i == g_mem_vma_count || g_mem_vmas[i].start > at ||
            (!allow_reserved && g_mem_vmas[i].kind == KIND_RESERVED)) return 0;
        at = g_mem_vmas[i].end;
    }
    return 1;
}

int r_mem_overlaps(uintptr_t start, uintptr_t end, int ignore_reserved) {
    for (size_t i = r_mem_vma_index(start); i < g_mem_vma_count && g_mem_vmas[i].start < end; ++i)
        if (!ignore_reserved || g_mem_vmas[i].kind != KIND_RESERVED) return 1;
    return 0;
}

uintptr_t r_mem_find_free(uintptr_t hint, uint64_t size, uint64_t alignment) {
    uintptr_t at = (hint < USER_MIN ? USER_MIN : hint + alignment - 1) & ~(alignment - 1);
    for (size_t i = r_mem_vma_index(at); at + size <= USER_MAX; ++i) {
        if (i == g_mem_vma_count || g_mem_vmas[i].start >= at + size) return at;
        at = (g_mem_vmas[i].end + alignment - 1) & ~(alignment - 1);
    }
    return 0;
}

void r_mem_drop_range(uintptr_t start, uintptr_t end) {
    int error; size_t i = r_mem_carve(start, end, &error), n = 0;
    while (i + n < g_mem_vma_count && g_mem_vmas[i + n].end <= end && g_mem_vmas[i + n].start >= start) {
        if (g_mem_vmas[i + n].kind == KIND_FLEXIBLE) {
            uint64_t len = g_mem_vmas[i + n].end - g_mem_vmas[i + n].start;
            g_mem_flex_bytes -= len;
            r_mem_flex_free(g_mem_vmas[i + n].phys, len);
        }
        ++n;
    }
    r_mem_vma_erase(i, n);
}

int32_t r_mem_place(void **inout, uint64_t size, int prot, int flags,
                    uint64_t alignment, int kind, int type, uint64_t phys) {
    uintptr_t address = (uintptr_t)*inout;
    if (flags & MAP_FIXED_FLAG) {
        if (!address || address % PAGE) return INVALID;
        if ((flags & MAP_NO_OVERWRITE) && r_mem_overlaps(address, address + size, 1)) return NO_MEMORY;
    } else {
        address = r_mem_find_free(address, size, alignment);
        if (!address) return NO_MEMORY;
    }
    int res = kind != KIND_RESERVED
        ? bb_platform_direct_map(r_mem_pool(), (void*)address, phys, size, prot)
        : (bb_platform_vm_reserve(address, size) ? 0 : -1);
    if (res != 0) return NO_MEMORY;
    r_mem_drop_range(address, address + size);
    if (r_mem_vma_insert(r_mem_vma_index(address), (Vma){address, address + size, kind, prot, type, phys}))
        return NO_MEMORY;
    if (kind == KIND_FLEXIBLE) g_mem_flex_bytes += size;
    if (kind != KIND_RESERVED) r_mem_queue_hook(HOOK_MAP, address, size);
    *inout = (void*)address;
    return 0;
}
