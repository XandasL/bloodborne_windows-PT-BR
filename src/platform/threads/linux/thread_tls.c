/* SPDX-License-Identifier: MIT
 * Linux thread-local storage anchor using arch_prctl(ARCH_SET_GS).
 * Single responsibility: Linux GS base manipulation for guest TCB. (~55 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/threads.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <asm/prctl.h>
#include <stdio.h>
#include <stdlib.h>

static _Thread_local void* cached_tcb = NULL;

void bb_platform_tls_set_guest(void* tcb) {
    cached_tcb = tcb;
    if (syscall(SYS_arch_prctl, ARCH_SET_GS, (unsigned long)tcb) != 0) {
        perror("bb_platform_tls_set_guest: arch_prctl failed");
        exit(21);
    }
}

void* bb_platform_tls_get_guest(void) {
    return cached_tcb;
}
#endif
