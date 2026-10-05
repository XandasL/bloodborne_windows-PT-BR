/* SPDX-License-Identifier: MIT
 * PS4 Virtual Query and Batch Mapping Operations.
 * Single responsibility: Memory region metadata lookup and batching. (~75 LOC)
 */
#include "r_mem_types.h"

size_t g_mem_queries = 0;

int32_t r_mem_map_direct_locked(void **out, uint64_t size, int prot, int flags, int64_t phys, uint64_t align);
int32_t r_mem_unmap_locked(uintptr_t start, uint64_t size);
int32_t r_mem_protect_locked(uintptr_t start, uint64_t size, int prot, int type);
int32_t r_mem_map_flex_locked(void **inout, uint64_t size, int prot, int flags);

ABI int32_t virtual_query(const void *address, int flags, VirtualQueryInfo *info, uint64_t info_size) {
    if (!info || info_size != sizeof(*info)) return INVALID;
    r_mem_read_lock();
    size_t i = r_mem_vma_index((uintptr_t)address);
    if (i == g_mem_vma_count || (g_mem_vmas[i].start > (uintptr_t)address && !(flags & 1))) {
        r_mem_read_unlock(); return ACCESS;
    }
    Vma v = g_mem_vmas[i]; memset(info, 0, sizeof(*info));
    info->start = v.start; info->end = v.end;
    info->offset = v.kind == KIND_DIRECT ? v.phys : 0;
    info->protection = v.prot; info->memory_type = v.type;
    info->flags = (v.kind == KIND_FLEXIBLE ? 1u : 0) | (v.kind == KIND_DIRECT ? 2u : 0) | (v.kind != KIND_RESERVED ? 16u : 0);
    strcpy(info->name, v.kind == KIND_RESERVED ? "reserved" : v.kind == KIND_DIRECT ? "direct" : "flexible");
    __atomic_add_fetch(&g_mem_queries, 1, __ATOMIC_RELAXED);
    r_mem_read_unlock();
    return 0;
}

ABI int32_t direct_memory_type(uint64_t phys, int *type, void **start, void **end) {
    r_mem_read_lock();
    for (int i = 0; i < LIMIT; ++i) {
        if (g_mem_blocks[i].used && phys >= g_mem_blocks[i].start &&
            phys - g_mem_blocks[i].start < g_mem_blocks[i].size) {
            if (type) *type = g_mem_blocks[i].type;
            if (start) *start = (void*)(uintptr_t)g_mem_blocks[i].start;
            if (end) *end = (void*)(uintptr_t)(g_mem_blocks[i].start + g_mem_blocks[i].size);
            r_mem_read_unlock(); return 0;
        }
    }
    r_mem_read_unlock(); return ACCESS;
}

ABI int32_t batch_map2(BatchEntry *entries, int count, int *processed, int flags) {
    if (!entries || count < 0) return INVALID;
    int32_t res = 0; int done = 0;
    r_mem_write_lock();
    for (; done < count; ++done) {
        BatchEntry *e = &entries[done];
        switch (e->operation) {
        case 0: res = r_mem_map_direct_locked(&e->start, e->length, (uint8_t)e->prot, flags, (int64_t)e->offset, 0); break;
        case 1: res = r_mem_unmap_locked((uintptr_t)e->start, e->length); break;
        case 2: res = r_mem_protect_locked((uintptr_t)e->start, e->length, (uint8_t)e->prot, -1); break;
        case 3: res = r_mem_map_flex_locked(&e->start, e->length, (uint8_t)e->prot, flags); break;
        case 4: res = r_mem_protect_locked((uintptr_t)e->start, e->length, (uint8_t)e->prot, e->type); break;
        default: res = INVALID;
        }
        if (res) break;
    }
    r_mem_write_unlock(); r_mem_flush_hooks();
    if (processed) *processed = done;
    return res;
}

ABI int32_t batch_map(BatchEntry *entries, int count, int *processed) {
    return batch_map2(entries, count, processed, MAP_FIXED_FLAG);
}
