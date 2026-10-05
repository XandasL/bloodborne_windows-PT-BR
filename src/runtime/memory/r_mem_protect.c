/* SPDX-License-Identifier: MIT
 * PS4 Virtual Memory Protection and GPU Page Tracking.
 * Single responsibility: Page permission manipulation and protection queries. (~75 LOC)
 */
#include "r_mem_types.h"

size_t g_mem_protects = 0;

int32_t r_mem_protect_locked(uintptr_t start, uint64_t size, int prot, int type) {
    uintptr_t end = start + ((size + PAGE - 1) & ~(PAGE - 1));
    start &= ~(PAGE - 1);
    if ((prot & ~0x37) || !r_mem_covered(start, end, 0)) return INVALID;
    if (bb_platform_vm_protect((void*)start, end - start, prot) != 0) return INVALID;
    int error; size_t i = r_mem_carve(start, end, &error);
    if (error) return NO_MEMORY;
    for (; i < g_mem_vma_count && g_mem_vmas[i].start < end; ++i) {
        g_mem_vmas[i].prot = prot;
        if (type >= 0) g_mem_vmas[i].type = type;
    }
    r_mem_queue_hook(HOOK_INVALIDATE, start, end - start);
    ++g_mem_protects;
    return 0;
}

ABI int32_t kernel_mprotect(const void *address, uint64_t size, int prot) {
    r_mem_write_lock();
    int32_t res = r_mem_protect_locked((uintptr_t)address, size, prot, -1);
    r_mem_write_unlock();
    r_mem_flush_hooks();
    return res;
}

ABI int32_t query_protection(void *address, void **start, void **end, uint32_t *prot) {
    r_mem_read_lock();
    size_t i = r_mem_vma_index((uintptr_t)address);
    if (i == g_mem_vma_count || g_mem_vmas[i].start > (uintptr_t)address || g_mem_vmas[i].kind == KIND_RESERVED) {
        r_mem_read_unlock(); return ACCESS;
    }
    size_t first = i, last = i;
    while (first && g_mem_vmas[first-1].end == g_mem_vmas[first].start &&
           g_mem_vmas[first-1].kind != KIND_RESERVED && g_mem_vmas[first-1].prot == g_mem_vmas[i].prot) --first;
    while (last + 1 < g_mem_vma_count && g_mem_vmas[last+1].start == g_mem_vmas[last].end &&
           g_mem_vmas[last+1].kind != KIND_RESERVED && g_mem_vmas[last+1].prot == g_mem_vmas[i].prot) ++last;
    if (start) *start = (void*)g_mem_vmas[first].start;
    if (end) *end = (void*)g_mem_vmas[last].end;
    if (prot) *prot = (uint32_t)g_mem_vmas[i].prot;
    __atomic_add_fetch(&g_mem_queries, 1, __ATOMIC_RELAXED);
    r_mem_read_unlock();
    return 0;
}

void runtime_memory_gpu_protect(uintptr_t address, uint64_t size, int read, int write) {
    r_mem_read_lock();
    for (size_t i = r_mem_vma_index(address); i < g_mem_vma_count && g_mem_vmas[i].start < address + size; ++i) {
        if (g_mem_vmas[i].kind == KIND_RESERVED) continue;
        uintptr_t a = g_mem_vmas[i].start > address ? g_mem_vmas[i].start : address;
        uintptr_t b = g_mem_vmas[i].end < address + size ? g_mem_vmas[i].end : address + size;
        int guest = g_mem_vmas[i].prot;
        int want = (read ? BB_PROT_READ : 0) | (write ? BB_PROT_WRITE | BB_PROT_READ : 0) | (guest & BB_PROT_EXEC);
        bb_platform_vm_protect((void*)a, b - a, guest & want);
    }
    r_mem_read_unlock();
}
