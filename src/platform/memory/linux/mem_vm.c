/* SPDX-License-Identifier: MIT
 * Linux virtual memory management using mmap/mprotect.
 * Single responsibility: POSIX anonymous virtual memory operations. (~65 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/memory.h"
#include <sys/mman.h>
#include <unistd.h>

static int ToPosixProt(int prot) {
    int p = PROT_NONE;
    if (prot & BB_PROT_READ) p |= PROT_READ;
    if (prot & BB_PROT_WRITE) p |= PROT_WRITE;
    if (prot & BB_PROT_EXEC) p |= PROT_EXEC;
    return p;
}

void* bb_platform_vm_reserve(uintptr_t preferred_base, size_t size) {
    void* p = mmap((void*)preferred_base, size, PROT_NONE,
                   MAP_PRIVATE | MAP_ANONYMOUS | (preferred_base ? MAP_FIXED_NOREPLACE : 0), -1, 0);
    return (p == MAP_FAILED) ? NULL : p;
}

int bb_platform_vm_commit(void* addr, size_t size, int prot) {
    return mprotect(addr, size, ToPosixProt(prot));
}

int bb_platform_vm_decommit(void* addr, size_t size) {
    return mprotect(addr, size, PROT_NONE);
}

int bb_platform_vm_protect(void* addr, size_t size, int prot) {
    return mprotect(addr, size, ToPosixProt(prot));
}

int bb_platform_vm_release(void* addr, size_t size) {
    return munmap(addr, size);
}

size_t bb_platform_page_size(void) {
    return (size_t)sysconf(_SC_PAGESIZE);
}
#endif
