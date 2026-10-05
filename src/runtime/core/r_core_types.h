/* SPDX-License-Identifier: MIT
 * PS4 Core Runtime Types and Declarations.
 * Single responsibility: Core HLE definitions, exit handlers, and TLS constants. (~50 LOC)
 */
#ifndef R_CORE_TYPES_H
#define R_CORE_TYPES_H

#include "runtime.h"
#include "platform/time.h"
#include "platform/sync.h"
#include "platform/threads.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

#define TLS_MODULES 8

typedef struct {
    union { GuestCallback plain; void (ABI *with_arg)(void *); } callback;
    void *argument, *dso;
    int with_arg, active;
} ExitHandler;

extern uint64_t g_runtime_capabilities;
extern uint64_t g_runtime_stack_canary;
extern _Atomic size_t g_runtime_memory_calls;
extern _Atomic size_t g_runtime_guards_acquired;
extern _Atomic size_t g_runtime_guards_released;
extern size_t g_runtime_calls_init;
extern size_t g_runtime_calls_atexit;
extern size_t g_runtime_calls_cxa;
extern size_t g_runtime_handler_count;

void r_core_exit_init(void);

#endif /* R_CORE_TYPES_H */
