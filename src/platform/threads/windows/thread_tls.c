/* SPDX-License-Identifier: MIT
 * Windows thread-local storage anchor for guest TCB.
 * Single responsibility: Safe TLS slot storage for guest context. (~45 LOC)
 */
#ifdef _WIN32
#include "platform/threads.h"
#include <windows.h>

static DWORD tls_slot = TLS_OUT_OF_INDEXES;

static void ensure_tls_slot(void) {
    if (tls_slot == TLS_OUT_OF_INDEXES) {
        tls_slot = TlsAlloc();
    }
}

void bb_platform_tls_set_guest(void* tcb) {
    ensure_tls_slot();
    TlsSetValue(tls_slot, tcb);
}

void* bb_platform_tls_get_guest(void) {
    ensure_tls_slot();
    return TlsGetValue(tls_slot);
}
#endif
