/* SPDX-License-Identifier: MIT
 * Windows direct memory physical aliasing using MapViewOfFileEx.
 * Single responsibility: Multi-view aliased mapping to shared section. (~75 LOC)
 */
#ifdef _WIN32
#include "platform/memory.h"
#include <windows.h>

struct BbDirectPool {
    HANDLE section_handle;
    size_t pool_size;
};

int bb_platform_direct_map(BbDirectPool* pool, void* vaddr,
                           uint64_t phys_offset, size_t size, int prot) {
    if (!pool || !pool->section_handle || !vaddr) return -1;
    DWORD access = FILE_MAP_READ;
    if (prot & BB_PROT_WRITE) access |= FILE_MAP_WRITE;
    if (prot & BB_PROT_EXEC) access |= FILE_MAP_EXECUTE;

    /* If vaddr is currently reserved, unreserve the range before mapping the view */
    VirtualFree(vaddr, 0, MEM_RELEASE);

    void* mapped = MapViewOfFileEx(
        pool->section_handle, access,
        (DWORD)(phys_offset >> 32), (DWORD)(phys_offset & 0xFFFFFFFF),
        size, vaddr
    );

    return (mapped == vaddr) ? 0 : -1;
}

int bb_platform_direct_unmap(void* vaddr, size_t size) {
    BB_UNUSED(size);
    if (!UnmapViewOfFile(vaddr)) return -1;
    /* Re-reserve the unmapped region to prevent host allocations from occupying it */
    VirtualAlloc(vaddr, size, MEM_RESERVE, PAGE_NOACCESS);
    return 0;
}
#endif
