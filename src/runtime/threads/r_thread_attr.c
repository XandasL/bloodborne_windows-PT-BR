/* SPDX-License-Identifier: MIT
 * PS4 Thread Attribute Allocation and Lifecycle.
 * Single responsibility: Attribute allocation, lookup, and destruction. (~45 LOC)
 */
#include "r_thread_types.h"

static ThreadAttr *attributes;

ThreadAttr *r_thread_find_attr(ThreadAttr **slot) {
    if (!slot || !*slot) return NULL;
    ThreadAttr *found = NULL;
    r_thread_lock();
    for (ThreadAttr *a = attributes; a; a = a->next)
        if (a == *slot && a->magic == ATTR_MAGIC) { found = a; break; }
    r_thread_unlock();
    return found;
}

ABI int32_t attr_init(ThreadAttr **out) {
    if (!out) return ERR(22);
    ThreadAttr *a = calloc(1, sizeof(*a));
    if (!a) return ERR(12);
    *a = (ThreadAttr){.magic = ATTR_MAGIC, .policy = 1, .prio = DEFAULT_PRIO, .inherit = 4, .stack = DEFAULT_STACK, .guard = 4096, .affinity = 0x7f};
    r_thread_lock(); a->next = attributes; attributes = a; r_thread_unlock();
    *out = a; return 0;
}

ABI int32_t attr_destroy(ThreadAttr **slot) {
    ThreadAttr *a = r_thread_find_attr(slot);
    if (!a) return ERR(22);
    r_thread_lock();
    ThreadAttr **link = &attributes;
    while (*link != a) link = &(*link)->next;
    *link = a->next; a->magic = 0;
    r_thread_unlock();
    free(a); *slot = NULL; return 0;
}

ABI int32_t attr_get(void *thread, ThreadAttr **out) {
    ThreadAttr *a = r_thread_find_attr(out);
    if (!a || !thread) return ERR(22);
    GuestThread *t = r_thread_find(thread);
    if (!t) return ERR(3);
    ThreadAttr *next = a->next;
    *a = t->attr; a->magic = ATTR_MAGIC; a->next = next; a->detached = t->detached;
    return 0;
}
