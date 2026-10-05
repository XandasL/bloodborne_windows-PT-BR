/* SPDX-License-Identifier: MIT
 * PS4 Thread Lifecycle and Creation.
 * Single responsibility: Thread creation, proxy entry, and list management. (~65 LOC)
 */
#include "r_thread_types.h"

static GuestThread *threads;
static BbMutex *thread_lock;
size_t g_threads_created, g_threads_joined, g_threads_exited;

static void ensure_lock(void) {
    if (!thread_lock) {
        BbMutex *m = bb_platform_mutex_create(BB_MUTEX_TYPE_NORMAL);
        if (!__sync_bool_compare_and_swap(&thread_lock, NULL, m)) bb_platform_mutex_destroy(m);
    }
}
void r_thread_lock(void) { ensure_lock(); bb_platform_mutex_lock(thread_lock); }
void r_thread_unlock(void) { if (thread_lock) bb_platform_mutex_unlock(thread_lock); }

GuestThread *r_thread_find(void *handle) {
    GuestThread *found = NULL;
    r_thread_lock();
    for (GuestThread *t = threads; t; t = t->next) if (t == handle) { found = t; break; }
    r_thread_unlock();
    return found;
}
void r_thread_publish(GuestThread *t) { r_thread_lock(); t->next = threads; threads = t; r_thread_unlock(); }
GuestThread *r_thread_new(void) {
    GuestThread *t = (GuestThread*)calloc(1, sizeof(*t));
    if (t) t->attr = (ThreadAttr){.magic = ATTR_MAGIC, .policy = 1, .prio = DEFAULT_PRIO, .stack = DEFAULT_STACK, .affinity = 0x7f};
    return t;
}

void runtime_thread_attach_host(const char *name) {
    snprintf(runtime_thread_current()->name, sizeof(threads->name), "%s", name ? name : "host");
}
void runtime_thread_attach_main(void) {
    snprintf(runtime_thread_current()->name, sizeof(threads->name), "main");
}

static void *host_start(void *p) {
    GuestThread *t = (GuestThread*)p;
    r_thread_attach(t);
    if (!setjmp(t->exit_jump)) t->result = t->entry(t->argument);
    runtime_thread_keys_cleanup();
    r_thread_lock(); t->finished = 1; ++g_threads_exited; r_thread_unlock();
    return t->result;
}

int32_t r_thread_create_impl(GuestThread **out, ThreadAttr **attr_slot, GuestEntry entry, void *argument, const char *name) {
    if (!out || !entry) return ERR(22);
    ThreadAttr *a = NULL;
    if (attr_slot && !(a = r_thread_find_attr(attr_slot))) return ERR(22);
    GuestThread *t = r_thread_new();
    if (!t) return ERR(12);
    if (a) { t->attr = *a; t->attr.next = NULL; }
    t->entry = entry; t->argument = argument; t->detached = t->attr.detached;
    snprintf(t->name, sizeof(t->name), "%s", name ? name : "guest");
    uint64_t stack = t->attr.stack < MIN_STACK ? MIN_STACK : t->attr.stack;
    r_thread_publish(t);
    *out = t;
    t->host = bb_platform_thread_create(host_start, t, (size_t)stack + STACK_MARGIN, t->name);
    if (!t->host) { fprintf(stderr, "STOP: host thread create failed\n"); exit(21); }
    if (t->detached) bb_platform_thread_detach(t->host);
    r_thread_lock(); ++g_threads_created; r_thread_unlock();
    return 0;
}

ABI int32_t thread_create(GuestThread **out, ThreadAttr **attr, GuestEntry entry, void *arg, const char *name) {
    return r_thread_create_impl(out, attr, entry, arg, name);
}
