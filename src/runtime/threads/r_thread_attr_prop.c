/* SPDX-License-Identifier: MIT
 * PS4 Thread Attribute Properties.
 * Single responsibility: Attribute stack, policy, affinity, and param accessors. (~55 LOC)
 */
#include "r_thread_types.h"

ABI int32_t attr_set_stack(ThreadAttr **slot, uint64_t size) {
    ThreadAttr *a = r_thread_find_attr(slot);
    if (!a || size < 16384) return ERR(22);
    a->stack = size; return 0;
}
ABI int32_t attr_get_stack(ThreadAttr **slot, uint64_t *size) {
    ThreadAttr *a = r_thread_find_attr(slot);
    if (!a || !size) return ERR(22);
    *size = a->stack; return 0;
}
ABI int32_t attr_set_detach(ThreadAttr **s, int st) {
    ThreadAttr *a = r_thread_find_attr(s); if (!a || (st != 0 && st != 1)) return ERR(22);
    a->detached = st; return 0;
}
ABI int32_t attr_get_detach(ThreadAttr **s, int *st) {
    ThreadAttr *a = r_thread_find_attr(s); if (!a || !st) return ERR(22);
    *st = a->detached; return 0;
}
ABI int32_t attr_set_policy(ThreadAttr **s, int p) {
    ThreadAttr *a = r_thread_find_attr(s); if (!a || p < 1 || p > 3) return ERR(22);
    a->policy = p; return 0;
}
ABI int32_t attr_set_inherit(ThreadAttr **s, int in) {
    ThreadAttr *a = r_thread_find_attr(s); if (!a || (in != 0 && in != 4)) return ERR(22);
    a->inherit = in; return 0;
}
ABI int32_t attr_set_param(ThreadAttr **s, const int *p) {
    ThreadAttr *a = r_thread_find_attr(s); if (!a || !p) return ERR(22);
    a->prio = *p; return 0;
}
ABI int32_t attr_get_param(ThreadAttr **s, int *p) {
    ThreadAttr *a = r_thread_find_attr(s); if (!a || !p) return ERR(22);
    *p = a->prio; return 0;
}
ABI int32_t attr_set_affinity(ThreadAttr **s, uint64_t m) {
    ThreadAttr *a = r_thread_find_attr(s); if (!a) return ERR(22);
    a->affinity = m; return 0;
}
ABI int32_t attr_affinity(ThreadAttr **s, uint64_t *m) {
    ThreadAttr *a = r_thread_find_attr(s); if (!a || !m) return ERR(22);
    *m = a->affinity; return 0;
}
ABI int32_t attr_set_guard(ThreadAttr **s, uint64_t sz) {
    ThreadAttr *a = r_thread_find_attr(s); if (!a) return ERR(22);
    a->guard = sz; return 0;
}
