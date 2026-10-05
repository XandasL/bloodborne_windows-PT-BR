/* SPDX-License-Identifier: MIT
 * PS4 Core Process Exit and Finalizer Handlers.
 * Single responsibility: atexit/cxa_atexit registration and ordered execution. (~65 LOC)
 */
#include "r_core_types.h"

static ExitHandler *handlers = NULL;
size_t g_runtime_handler_count = 0;
static size_t handler_capacity = 0;
size_t g_runtime_calls_init = 0;
size_t g_runtime_calls_atexit = 0;
size_t g_runtime_calls_cxa = 0;
static BbMutex *handler_lock = NULL;

void r_core_exit_init(void) {
    if (!handler_lock) handler_lock = bb_platform_mutex_create(BB_MUTEX_TYPE_NORMAL);
}

ABI void guest_init_env(void) {
    ++g_runtime_calls_init;
    puts("Runtime: _init_env returned (verified libc implementation: RET)");
}

static int register_handler(ExitHandler value) {
    r_core_exit_init();
    bb_platform_mutex_lock(handler_lock);
    if (g_runtime_handler_count == handler_capacity) {
        size_t capacity = handler_capacity ? handler_capacity * 2 : 64;
        ExitHandler *next = (ExitHandler *)realloc(handlers, capacity * sizeof(*handlers));
        if (!next) { bb_platform_mutex_unlock(handler_lock); return -1; }
        handlers = next; handler_capacity = capacity;
    }
    value.active = 1;
    handlers[g_runtime_handler_count++] = value;
    bb_platform_mutex_unlock(handler_lock);
    return 0;
}

ABI int guest_atexit(GuestCallback cb) {
    if (!cb) return -1;
    ++g_runtime_calls_atexit;
    return register_handler((ExitHandler){.callback.plain = cb});
}

ABI int guest_cxa_atexit(void (ABI *cb)(void *), void *arg, void *dso) {
    if (!cb) return -1;
    ++g_runtime_calls_cxa;
    return register_handler((ExitHandler){.callback.with_arg = cb, .argument = arg, .dso = dso, .with_arg = 1});
}

void runtime_finalize(void *dso) {
    r_core_exit_init();
    for (;;) {
        bb_platform_mutex_lock(handler_lock);
        size_t i = g_runtime_handler_count;
        while (i && (!handlers[i - 1].active || (dso && handlers[i - 1].dso != dso))) --i;
        if (!i) { bb_platform_mutex_unlock(handler_lock); return; }
        ExitHandler h = handlers[i - 1];
        handlers[i - 1].active = 0;
        bb_platform_mutex_unlock(handler_lock);
        if (h.with_arg) h.callback.with_arg(h.argument);
        else h.callback.plain();
    }
}

ABI void guest_finalize(void *dso) { runtime_finalize(dso); }
