/* SPDX-License-Identifier: MIT
 * PS4 Direct Memory Virtual Mapping and Unmapping Handlers.
 * Single responsibility: Direct range mapping and physical release. (~65 LOC)
 */
#include "r_mem_types.h"

int direct_range_allocated(uint64_t phys, uint64_t size);

int32_t r_mem_map_direct_locked(void **out, uint64_t size, int prot, int flags, int64_t phys, uint64_t align) {
    if (!align) align = PAGE;
    if (!out || phys < 0 || (uint64_t)phys % PAGE || !size || size % PAGE ||
        (prot & ~0x37) || !direct_range_allocated((uint64_t)phys, size)) return INVALID;
    int32_t res = r_mem_place(out, size, prot, flags, align, KIND_DIRECT, 0, (uint64_t)phys);
    if (!res) ++g_mem_maps;
    return res;
}

ABI int32_t direct_map(void **out, uint64_t size, int prot, int flags, int64_t phys, uint64_t align) {
    r_mem_write_lock();
    int32_t res = r_mem_map_direct_locked(out, size, prot, flags, phys, align);
    r_mem_write_unlock(); r_mem_flush_hooks();
    return res;
}

ABI int32_t direct_map_named(void **o, uint64_t s, int p, int f, int64_t ph, uint64_t a, const char *n) {
    (void)n; return direct_map(o, s, p, f, ph, a);
}

int32_t r_mem_unmap_locked(uintptr_t start, uint64_t size) {
    if (start % PAGE || !size) return INVALID;
    uint64_t end = start + ((size + PAGE - 1) & ~(PAGE - 1));
    for (size_t i = r_mem_vma_index(start); i < g_mem_vma_count && g_mem_vmas[i].start < end; ++i) {
        if (g_mem_vmas[i].kind != KIND_RESERVED) {
            uintptr_t a = g_mem_vmas[i].start > start ? g_mem_vmas[i].start : start;
            uintptr_t b = g_mem_vmas[i].end < end ? g_mem_vmas[i].end : end;
            r_mem_queue_hook(HOOK_UNMAP, a, b - a);
            bb_platform_direct_unmap((void*)a, b - a);
        }
    }
    r_mem_drop_range(start, end);
    return 0;
}

ABI int32_t direct_unmap(void *address, uint64_t size) {
    r_mem_write_lock();
    int32_t res = r_mem_unmap_locked((uintptr_t)address, size);
    r_mem_write_unlock(); r_mem_flush_hooks();
    return res;
}

ABI int32_t direct_release(uint64_t start, uint64_t size) {
    if (start % PAGE || size % PAGE) return INVALID;
    if (!size) return 0;
    r_mem_write_lock();
    if (!direct_range_allocated(start, size)) { r_mem_write_unlock(); return INVALID; }
    for (size_t i = 0; i < g_mem_vma_count;) {
        Vma v = g_mem_vmas[i]; uint64_t len = v.end - v.start;
        if (v.kind == KIND_DIRECT && v.phys < start + size && start < v.phys + len) {
            uint64_t a = v.phys > start ? v.phys : start, b = v.phys + len < start + size ? v.phys + len : start + size;
            r_mem_unmap_locked(v.start + (a - v.phys), b - a);
            i = r_mem_vma_index(v.start); continue;
        }
        ++i;
    }
    for (int i = 0; i < LIMIT; ++i) {
        Block *b = &g_mem_blocks[i];
        if (!b->used || b->start >= start + size || start >= b->start + b->size) continue;
        uint64_t a = b->start > start ? b->start : start, e = b->start + b->size < start + size ? b->start + b->size : start + size;
        b->used = 0; g_mem_live_bytes -= e - a;
    }
    bb_platform_direct_punch_hole(r_mem_pool(), start, size);
    r_mem_write_unlock(); r_mem_flush_hooks();
    return 0;
}
