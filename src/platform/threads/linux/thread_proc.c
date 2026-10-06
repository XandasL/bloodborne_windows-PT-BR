/* SPDX-License-Identifier: MIT
 * Linux process control and restart implementation.
 * Single responsibility: Process recreation and clean exec handoff. (~35 LOC)
 */
#ifndef _WIN32
#include "platform/threads.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <stdio.h>

void bb_platform_thread_set_name(BbThread* thread, const char* name) {
    (void)thread;
    (void)name;
}

void bb_platform_process_restart(void) {
#ifdef SYS_close_range
    syscall(SYS_close_range, 3u, ~0u, 0u);
#endif
    execlp("bash", "bash", "run.sh", (char *)NULL);
    perror("bb_platform_process_restart: exec");
    _exit(1);
}
#endif
