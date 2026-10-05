/* SPDX-License-Identifier: MIT
 * PS4 Virtual Memory NID Exports and Status Reporting.
 * Single responsibility: NID dispatch table and diagnostic stats. (~75 LOC)
 */
#include "r_mem_types.h"

ABI uint64_t direct_size(void);
ABI int32_t direct_allocate(int64_t low, int64_t high, uint64_t size, uint64_t alignment, int type, int64_t *out);
ABI int32_t direct_map(void **out, uint64_t size, int prot, int flags, int64_t physical, uint64_t alignment);
ABI int32_t direct_map_named(void **out, uint64_t size, int prot, int flags, int64_t physical, uint64_t alignment, const char *name);
ABI int32_t direct_release(uint64_t start, uint64_t size);
ABI int32_t direct_unmap(void *address, uint64_t size);
ABI int32_t map_flexible(void **inout, uint64_t size, int prot, int flags);
ABI int32_t map_flexible_named(void **inout, uint64_t size, int prot, int flags, const char *name);
ABI int32_t release_flexible(void *address, uint64_t size);
ABI int32_t reserve_range(void **inout, uint64_t size, int flags, uint64_t alignment);
ABI int32_t kernel_mprotect(const void *address, uint64_t size, int prot);
ABI int32_t query_protection(void *address, void **start, void **end, uint32_t *prot);
ABI int32_t virtual_query(const void *address, int flags, VirtualQueryInfo *info, uint64_t info_size);
ABI int32_t direct_memory_type(uint64_t phys, int *type, void **start, void **end);
ABI int32_t batch_map(BatchEntry *entries, int count, int *processed);
ABI int32_t batch_map2(BatchEntry *entries, int count, int *processed, int flags);

static const RuntimeExport exports[] = {
    {"sceKernelGetDirectMemorySize", (void*)direct_size}, {"sceKernelAllocateDirectMemory", (void*)direct_allocate},
    {"sceKernelMapDirectMemory", (void*)direct_map}, {"sceKernelMapNamedDirectMemory", (void*)direct_map_named},
    {"sceKernelReleaseDirectMemory", (void*)direct_release}, {"sceKernelMunmap", (void*)direct_unmap}, {"munmap", (void*)direct_unmap},
    {"sceKernelMapFlexibleMemory", (void*)map_flexible}, {"sceKernelMapNamedFlexibleMemory", (void*)map_flexible_named},
    {"sceKernelReleaseFlexibleMemory", (void*)release_flexible}, {"sceKernelReserveVirtualRange", (void*)reserve_range},
    {"sceKernelMprotect", (void*)kernel_mprotect}, {"sceKernelQueryMemoryProtection", (void*)query_protection},
    {"sceKernelVirtualQuery", (void*)virtual_query}, {"sceKernelGetDirectMemoryType", (void*)direct_memory_type},
    {"sceKernelBatchMap", (void*)batch_map}, {"sceKernelBatchMap2", (void*)batch_map2},
};

uintptr_t runtime_memory_resolve(const char *name) {
    if (!strcmp(name, "pO96TwzOm5E#p#J")) return (uintptr_t)direct_size;
    if (!strcmp(name, "rTXw65xmLIA#p#J")) return (uintptr_t)direct_allocate;
    if (!strcmp(name, "L-Q3LEjIbgA#p#J")) return (uintptr_t)direct_map;
    if (!strcmp(name, "MBuItvba6z8#p#J")) return (uintptr_t)direct_release;
    if (!strcmp(name, "cQke9UuBQOk#p#J")) return (uintptr_t)direct_unmap;
    return RUNTIME_LOOKUP(exports, name);
}

void runtime_memory_report(void) {
    printf("Runtime: direct memory allocations=%zu, maps=%zu, live=%" PRIu64 ", budget=%" PRIu64 "\n",
           g_mem_allocations, g_mem_maps, g_mem_live_bytes, r_mem_pool_size());
    printf("Runtime: flexible maps=%zu, in use=%" PRIu64 ", protects=%zu, queries=%zu, regions=%zu\n",
           g_mem_flex_maps, g_mem_flex_bytes, g_mem_protects, g_mem_queries, g_mem_vma_count);
}
