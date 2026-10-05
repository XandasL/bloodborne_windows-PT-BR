/* SPDX-License-Identifier: MIT
 * Windows virtual memory address space reservation and protection.
 * Single responsibility: Win32 virtual memory primitives. (~70 LOC)
 */
#ifdef _WIN32
#include "platform/memory.h"
#include <windows.h>

static DWORD ToWin32Prot(int prot) {
    if ((prot & BB_PROT_READ) && (prot & BB_PROT_WRITE) && (prot & BB_PROT_EXEC)) return PAGE_EXECUTE_READWRITE;
    if ((prot & BB_PROT_READ) && (prot & BB_PROT_EXEC)) return PAGE_EXECUTE_READ;
    if ((prot & BB_PROT_READ) && (prot & BB_PROT_WRITE)) return PAGE_READWRITE;
    if (prot & BB_PROT_READ) return PAGE_READONLY;
    return PAGE_NOACCESS;
}

void* bb_platform_vm_reserve(uintptr_t preferred_base, size_t size) {
    return VirtualAlloc((void*)preferred_base, size, MEM_RESERVE, PAGE_NOACCESS);
}

int bb_platform_vm_commit(void* addr, size_t size, int prot) {
    void* p = VirtualAlloc(addr, size, MEM_COMMIT, ToWin32Prot(prot));
    return p ? 0 : -1;
}

int bb_platform_vm_decommit(void* addr, size_t size) {
    return VirtualFree(addr, size, MEM_DECOMMIT) ? 0 : -1;
}

int bb_platform_vm_protect(void* addr, size_t size, int prot) {
    DWORD old;
    return VirtualProtect(addr, size, ToWin32Prot(prot), &old) ? 0 : -1;
}

int bb_platform_vm_release(void* addr, size_t size) {
    BB_UNUSED(size);
    return VirtualFree(addr, 0, MEM_RELEASE) ? 0 : -1;
}

size_t bb_platform_page_size(void) {
    static size_t page_sz = 0;
    if (!page_sz) {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        page_sz = si.dwPageSize;
    }
    return page_sz;
}
#endif
