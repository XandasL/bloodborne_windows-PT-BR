/* SPDX-License-Identifier: MIT
 * Linux direct memory aliasing using shared mmap over memfd.
 * Single responsibility: Multi-virtual address mapping to single pool. (~65 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/memory.h"
#include "mem_internal.h"
#include <sys/mman.h>

static int ToPosixProt(int prot) {
    int p = PROT_NONE;
    if (prot & BB_PROT_READ) p |= PROT_READ;
    if (prot & BB_PROT_WRITE) p |= PROT_WRITE;
    if (prot & BB_PROT_EXEC) p |= PROT_EXEC;
    return p;
}

int bb_platform_direct_map(BbDirectPool* pool, void* vaddr,
                           uint64_t phys_offset, size_t size, int prot) {
    if (!pool || pool->fd < 0 || !vaddr) return -1;
    void* p = mmap(vaddr, size, ToPosixProt(prot),
                   MAP_SHARED | MAP_FIXED, pool->fd, (off_t)phys_offset);
    return (p == MAP_FAILED) ? -1 : 0;
}

int bb_platform_direct_unmap(void* vaddr, size_t size) {
    /* Replace mapping with anonymous PROT_NONE reservation to keep address reserved */
    void* p = mmap(vaddr, size, PROT_NONE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
    return (p == MAP_FAILED) ? -1 : 0;
}
#endif
