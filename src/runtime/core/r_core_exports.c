/* SPDX-License-Identifier: MIT
 * PS4 Core Runtime Lifetime and Reporting.
 * Single responsibility: Runtime startup canary and subsystem diagnostic reports. (~50 LOC)
 */
#include "r_core_types.h"

uint64_t g_runtime_capabilities = 0;
uint64_t g_runtime_stack_canary = 0;

void runtime_start(uint64_t flags) {
    g_runtime_capabilities = flags;
    if (!(flags & 1)) return;
    g_runtime_stack_canary = bb_platform_random_u64() & ~UINT64_C(255);
}

void runtime_report(void) {
    printf("Runtime: _init_env=%zu, atexit=%zu, __cxa_atexit=%zu, registered handlers=%zu\n",
           g_runtime_calls_init, g_runtime_calls_atexit, g_runtime_calls_cxa, g_runtime_handler_count);
    printf("Runtime: static guards acquired=%zu, released=%zu\n",
           (size_t)g_runtime_guards_acquired, (size_t)g_runtime_guards_released);
    runtime_thread_report();
    runtime_mutex_report();
    runtime_rwlock_report();
    runtime_sema_report();
    runtime_content_report();
    runtime_memory_report();
    runtime_ajm_report();
    runtime_audio_report();
    runtime_pad_report();
    runtime_savedata_report();
    runtime_file_report();
    printf("Runtime: memory operations=%zu\n", (size_t)g_runtime_memory_calls);
}
