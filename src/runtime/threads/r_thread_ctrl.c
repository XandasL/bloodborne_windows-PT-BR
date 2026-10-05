/* SPDX-License-Identifier: MIT
 * PS4 Thread Control Operations.
 * Single responsibility: Thread joining, detachment, priority, and exit. (~70 LOC)
 */
#include "r_thread_types.h"

ABI int32_t thread_join(GuestThread *t, void **result) {
    if (!r_thread_find(t) || t->host_owned) return ERR(3);
    if (t == runtime_thread_current()) return ERR(11);
    if (t->detached || t->joined) return ERR(22);
    t->joined = 1;
    void *val = NULL;
    int e = bb_platform_thread_join(t->host, &val);
    if (e) { fprintf(stderr, "STOP: host thread_join failed: %d\n", e); exit(21); }
    if (result) *result = t->result;
    r_thread_lock(); ++g_threads_joined; r_thread_unlock();
    return 0;
}

ABI int32_t thread_detach(GuestThread *t) {
    if (!r_thread_find(t)) return ERR(3);
    if (t->detached) return ERR(22);
    t->detached = 1;
    if (!t->host_owned) bb_platform_thread_detach(t->host);
    return 0;
}

ABI __attribute__((noreturn)) void thread_exit(void *value) {
    GuestThread *t = runtime_thread_current();
    if (t->host_owned) { fputs("STOP: thread_exit on host-owned/main thread\n", stderr); exit(21); }
    t->result = value;
    longjmp(t->exit_jump, 1);
}

ABI int32_t thread_yield(void) { bb_platform_thread_yield(); return 0; }

ABI int32_t thread_get_prio(GuestThread *t, int *prio) {
    if (!r_thread_find(t)) return ERR(3);
    if (!prio) return ERR(22);
    *prio = t->attr.prio; return 0;
}
ABI int32_t thread_set_prio(GuestThread *t, int prio) {
    if (!r_thread_find(t)) return ERR(3);
    t->attr.prio = prio; return 0;
}
ABI int32_t thread_set_affinity(GuestThread *t, uint64_t mask) {
    if (!r_thread_find(t)) return ERR(3);
    t->attr.affinity = mask; return 0;
}
ABI int32_t thread_get_affinity(GuestThread *t, uint64_t *mask) {
    if (!r_thread_find(t)) return ERR(3);
    if (!mask) return ERR(22);
    *mask = t->attr.affinity; return 0;
}
ABI int32_t thread_rename(GuestThread *t, const char *name) {
    if (!r_thread_find(t) || !name) return ERR(22);
    snprintf(t->name, sizeof(t->name), "%s", name);
    if (t->host) bb_platform_thread_set_name(t->host, t->name);
    return 0;
}
ABI int32_t thread_equal(GuestThread *a, GuestThread *b) { return a == b; }
