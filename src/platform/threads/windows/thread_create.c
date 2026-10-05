/* SPDX-License-Identifier: MIT
 * Windows thread creation and lifecycle implementation.
 * Single responsibility: _beginthreadex, thread joining, stack allocation. (~75 LOC)
 */
#ifdef _WIN32
#include "platform/threads.h"
#include <windows.h>
#include <process.h>
#include <stdlib.h>

struct BbThread {
    HANDLE handle;
    unsigned int tid;
    BbThreadEntry entry;
    void* arg;
    void* result;
};

static unsigned int __stdcall ThreadProxy(void* param) {
    BbThread* t = (BbThread*)param;
    t->result = t->entry(t->arg);
    return 0;
}

BbThread* bb_platform_thread_create(BbThreadEntry entry, void* arg, size_t stack_size, const char* name) {
    BbThread* t = (BbThread*)calloc(1, sizeof(BbThread));
    if (!t) return NULL;
    t->entry = entry;
    t->arg = arg;
    t->handle = (HANDLE)_beginthreadex(NULL, (unsigned int)stack_size, ThreadProxy, t, 0, &t->tid);
    if (!t->handle) { free(t); return NULL; }
    if (name) {
        wchar_t wname[64];
        MultiByteToWideChar(CP_UTF8, 0, name, -1, wname, 64);
        SetThreadDescription(t->handle, wname);
    }
    return t;
}

int32_t bb_platform_thread_join(BbThread* t, void** result) {
    if (!t || !t->handle) return BB_ORBIS_ERROR(3);
    WaitForSingleObject(t->handle, INFINITE);
    if (result) *result = t->result;
    CloseHandle(t->handle);
    free(t);
    return 0;
}

int32_t bb_platform_thread_detach(BbThread* t) {
    if (!t || !t->handle) return BB_ORBIS_ERROR(3);
    CloseHandle(t->handle);
    t->handle = NULL;
    return 0;
}

void bb_platform_thread_yield(void) {
    SwitchToThread();
}

uint64_t bb_platform_thread_id(void) {
    return (uint64_t)GetCurrentThreadId();
}

void* bb_platform_alloc_low_stack(size_t size) {
    return VirtualAlloc(NULL, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
}
#endif
