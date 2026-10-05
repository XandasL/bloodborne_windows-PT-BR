/* SPDX-License-Identifier: MIT
 * Windows process control and restart implementation.
 * Single responsibility: Process recreation and clean handoff. (~35 LOC)
 */
#ifdef _WIN32
#include "platform/threads.h"
#include <windows.h>
#include <process.h>
#include <stdlib.h>

void bb_platform_process_restart(void) {
    char exe[MAX_PATH];
    if (GetModuleFileNameA(NULL, exe, MAX_PATH) > 0) {
        _spawnl(_P_OVERLAY, exe, exe, NULL);
    }
    exit(1);
}
#endif
