/* SPDX-License-Identifier: MIT
 * PS4 Loader Guest Entry and Execution Transfer.
 * Single responsibility: Transferring control to guest entry point and restart handler. (~35 LOC)
 */
#include "loader_types.h"
#include "platform/threads.h"

static ABI void guest_exit(void) {
    puts("Runtime: process finalizer callback reached");
}

void enter_guest(void *entry_point, void *params, void *exit_fn) {
    g_entered_game = 1;
    typedef void (ABI *Entry)(void *, void (ABI *)(void));
    Entry entry = (Entry)entry_point;
    entry(params, exit_fn ? (void (ABI *)(void))exit_fn : guest_exit);
    fail("entry unexpectedly returned");
}

void runtime_restart(void) {
    fflush(NULL);
    puts("Runtime: restarting game process");
    bb_platform_process_restart();
}
