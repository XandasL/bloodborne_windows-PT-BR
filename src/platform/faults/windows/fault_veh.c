/* SPDX-License-Identifier: MIT
 * Windows Vectored Exception Handler (VEH) for GPU tracking & recovery.
 * Single responsibility: Exception dispatch for page tracking. (~75 LOC)
 */
#ifdef _WIN32
#include "platform/faults.h"
#include <windows.h>
#include <stdio.h>
#include <setjmp.h>

static BbFaultCallback gpu_callback = NULL;
static _Thread_local jmp_buf* recovery_point = NULL;

static LONG WINAPI VectoredHandler(PEXCEPTION_POINTERS ep) {
    if (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
        void* fault_addr = (void*)ep->ExceptionRecord->ExceptionInformation[1];
        if (gpu_callback && gpu_callback(ep->ContextRecord, fault_addr)) {
            return EXCEPTION_CONTINUE_EXECUTION;
        }
        if (recovery_point) {
            jmp_buf* rec = recovery_point;
            recovery_point = NULL;
            longjmp(*rec, 1);
        }
#if defined(_M_X64) || defined(__x86_64__)
        const ULONG_PTR access = ep->ExceptionRecord->ExceptionInformation[0];
        const char* kind = access == 0 ? "read" : access == 1 ? "write" :
                           access == 8 ? "execute" : "unknown";
        fprintf(stderr,
                "FATAL ACCESS VIOLATION: %s address=%p RIP=%p RSP=%p\n"
                "REGS: RAX=%p RCX=%p RDX=%p RDI=%p RSI=%p R8=%p R9=%p\n",
                kind, fault_addr,
                (void*)(uintptr_t)ep->ContextRecord->Rip,
                (void*)(uintptr_t)ep->ContextRecord->Rsp,
                (void*)(uintptr_t)ep->ContextRecord->Rax,
                (void*)(uintptr_t)ep->ContextRecord->Rcx,
                (void*)(uintptr_t)ep->ContextRecord->Rdx,
                (void*)(uintptr_t)ep->ContextRecord->Rdi,
                (void*)(uintptr_t)ep->ContextRecord->Rsi,
                (void*)(uintptr_t)ep->ContextRecord->R8,
                (void*)(uintptr_t)ep->ContextRecord->R9);
        fflush(stderr);
#endif
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

void bb_platform_faults_init(void) {
    AddVectoredExceptionHandler(1, VectoredHandler);
}

void bb_platform_faults_register_gpu(BbFaultCallback cb) {
    gpu_callback = cb;
}

void bb_platform_faults_set_recovery(void* jmp_buf_ptr) {
    recovery_point = (jmp_buf*)jmp_buf_ptr;
}

void* bb_platform_faults_get_recovery(void) {
    return recovery_point;
}

void bb_platform_dump_stack_trace(void* context) {
    BB_UNUSED(context);
    void* frames[32];
    WORD count = CaptureStackBackTrace(1, 32, frames, NULL);
    fprintf(stderr, "=== Host Crash Stack Trace (%d frames) ===\n", count);
    for (WORD i = 0; i < count; ++i) {
        fprintf(stderr, "  #%d: %p\n", i, frames[i]);
    }
}
#endif
