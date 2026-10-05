/* SPDX-License-Identifier: MIT
 * Windows thread-local storage anchor for guest TCB.
 * Single responsibility: Safe TLS storage for guest context. (~25 LOC)
 */
#ifdef _WIN32
#include "platform/threads.h"

static _Thread_local void* g_guest_tcb = NULL;

void bb_platform_tls_set_guest(void* tcb) {
    g_guest_tcb = tcb;
}

void* bb_platform_tls_get_guest(void) {
    return g_guest_tcb;
}
#endif
