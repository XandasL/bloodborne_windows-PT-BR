/* SPDX-License-Identifier: MIT
 * Linux signal fault handler for GPU page tracking & recovery.
 * Single responsibility: SIGSEGV/SIGBUS installation and dispatch. (~75 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/faults.h"
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <stdio.h>
#include <execinfo.h>

static BbFaultCallback gpu_callback = NULL;
static sigjmp_buf* recovery_point = NULL;

static void SignalHandler(int sig, siginfo_t* info, void* context) {
    if (sig == SIGSEGV && gpu_callback && gpu_callback(context, info->si_addr)) {
        return;
    }
    if ((sig == SIGSEGV || sig == SIGBUS) && recovery_point) {
        sigjmp_buf* rec = recovery_point;
        recovery_point = NULL;
        sigset_t unblock;
        sigemptyset(&unblock);
        sigaddset(&unblock, sig);
        pthread_sigmask(SIG_UNBLOCK, &unblock, NULL);
        siglongjmp(*rec, 1);
    }
    fprintf(stderr, "Host/Guest fault: signal %d at %p\n", sig, info->si_addr);
    bb_platform_dump_stack_trace(context);
    _exit(128 + sig);
}

void bb_platform_faults_init(void) {
    struct sigaction sa = {0};
    sa.sa_sigaction = SignalHandler;
    sa.sa_flags = SA_SIGINFO;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
}

void bb_platform_faults_register_gpu(BbFaultCallback cb) {
    gpu_callback = cb;
}

void bb_platform_faults_set_recovery(void* jmp_buf_ptr) {
    recovery_point = (sigjmp_buf*)jmp_buf_ptr;
}

void* bb_platform_faults_get_recovery(void) {
    return recovery_point;
}

void bb_platform_dump_stack_trace(void* context) {
    BB_UNUSED(context);
    void* frames[32];
    int depth = backtrace(frames, 32);
    for (int i = 0; i < depth; ++i) fprintf(stderr, "  #%d: %p\n", i, frames[i]);
}
#endif
