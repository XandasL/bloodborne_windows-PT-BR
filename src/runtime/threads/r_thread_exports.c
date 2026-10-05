/* SPDX-License-Identifier: MIT
 * PS4 Thread Subsystem Exports and POSIX Wrappers.
 * Single responsibility: NID resolution table and POSIX compatibility. (~70 LOC)
 */
#include "r_thread_types.h"

int32_t r_thread_create_impl(GuestThread **out, ThreadAttr **attr_slot, GuestEntry entry, void *argument, const char *name);
ABI void *thread_self(void) { return runtime_thread_current(); }
static _Thread_local int32_t guest_errno;
ABI int32_t *guest_error(void) { return &guest_errno; }
int32_t *runtime_errno(void) { return &guest_errno; }

ABI int32_t attr_init(ThreadAttr **out);
ABI int32_t attr_destroy(ThreadAttr **slot);
ABI int32_t attr_get(void *thread, ThreadAttr **out);
ABI int32_t attr_set_stack(ThreadAttr **slot, uint64_t size);
ABI int32_t attr_get_stack(ThreadAttr **slot, uint64_t *size);
ABI int32_t attr_set_detach(ThreadAttr **slot, int state);
ABI int32_t attr_get_detach(ThreadAttr **slot, int *state);
ABI int32_t attr_set_policy(ThreadAttr **slot, int policy);
ABI int32_t attr_set_inherit(ThreadAttr **slot, int inherit);
ABI int32_t attr_set_param(ThreadAttr **slot, const int *param);
ABI int32_t attr_get_param(ThreadAttr **slot, int *param);
ABI int32_t attr_set_affinity(ThreadAttr **slot, uint64_t mask);
ABI int32_t attr_affinity(ThreadAttr **slot, uint64_t *mask);
ABI int32_t attr_set_guard(ThreadAttr **slot, uint64_t size);
ABI int32_t thread_create(GuestThread **out, ThreadAttr **attr, GuestEntry entry, void *argument, const char *name);
ABI int32_t thread_join(GuestThread *t, void **result);
ABI int32_t thread_detach(GuestThread *t);
ABI __attribute__((noreturn)) void thread_exit(void *value);
ABI int32_t thread_yield(void);
ABI int32_t thread_get_prio(GuestThread *t, int *prio);
ABI int32_t thread_set_prio(GuestThread *t, int prio);
ABI int32_t thread_set_affinity(GuestThread *t, uint64_t mask);
ABI int32_t thread_get_affinity(GuestThread *t, uint64_t *mask);
ABI int32_t thread_rename(GuestThread *t, const char *name);
ABI int32_t thread_equal(GuestThread *a, GuestThread *b);

static int32_t posix(int32_t r) { return r ? (int32_t)((uint32_t)r & 0xffff) : 0; }
static ABI int32_t posix_attr_init(ThreadAttr **a) { return posix(attr_init(a)); }
static ABI int32_t posix_attr_destroy(ThreadAttr **a) { return posix(attr_destroy(a)); }
static ABI int32_t posix_attr_set_detach(ThreadAttr **a, int s) { return posix(attr_set_detach(a, s)); }
static ABI int32_t posix_attr_set_stack(ThreadAttr **a, uint64_t s) { return posix(attr_set_stack(a, s)); }
static ABI int32_t posix_attr_set_param(ThreadAttr **a, const int *p) { return posix(attr_set_param(a, p)); }
static ABI int32_t posix_create(GuestThread **t, ThreadAttr **a, GuestEntry e, void *arg) { return posix(r_thread_create_impl(t, a, e, arg, "posix")); }
static ABI int32_t posix_create_name(GuestThread **t, ThreadAttr **a, GuestEntry e, void *arg, const char *name) { return posix(r_thread_create_impl(t, a, e, arg, name)); }
static ABI int32_t posix_join(GuestThread *t, void **r) { return posix(thread_join(t, r)); }

uintptr_t runtime_thread_resolve(const char *name) {
    static const struct { const char *nid; void *fn; } table[] = {
        {"aI+OeCz8xrQ#p#J", thread_self}, {"EotR8a3ASf4#I#J", thread_self}, {"9BcDykPmo1I#p#J", guest_error},
        {"nsYoNRywwNg#p#J", attr_init}, {"62KCwEMmzcM#p#J", attr_destroy}, {"x1X76arYMxU#p#J", attr_get},
        {"8+s5BzZjxSg#p#J", attr_affinity}, {"UTXzJbWhhTE#p#J", attr_set_stack}, {"-Wreprtu0Qs#p#J", attr_set_detach},
        {"4+h9EzwKF4I#p#J", attr_set_policy}, {"DzES9hQF4f4#p#J", attr_set_param}, {"3qxgM4ezETA#p#J", attr_set_affinity},
        {"eXbUSpEaTsA#p#J", attr_set_inherit}, {"JaRMy+QcpeU#p#J", attr_get_detach}, {"-fA+7ZlGDQs#p#J", attr_get_stack},
        {"FXPWHNk8Of0#p#J", attr_get_param}, {"El+cQ20DynU#p#J", attr_set_guard}, {"6UgtwV+0zb4#p#J", thread_create},
        {"onNY9Byn-W8#p#J", thread_join}, {"4qGrR6eoP9Y#p#J", thread_detach}, {"3kg7rT0NQIs#p#J", thread_exit},
        {"T72hz6ffq08#p#J", thread_yield}, {"1tKyG7RlMJo#p#J", thread_get_prio}, {"W0Hpm2X0uPE#p#J", thread_set_prio},
        {"bt3CTBKmGyI#p#J", thread_set_affinity}, {"rcrVFJsQWRY#p#J", thread_get_affinity},
        {"GBUY7ywdULE#p#J", thread_rename}, {"3PtV6p3QNX4#p#J", thread_equal},
        {"wtkt-teR1so#I#J", posix_attr_init}, {"zHchY8ft5pk#I#J", posix_attr_destroy},
        {"E+tyo3lp5Lw#I#J", posix_attr_set_detach}, {"2Q0z6rnBrTE#I#J", posix_attr_set_stack},
        {"euKRgm0Vn2M#I#J", posix_attr_set_param}, {"OxhIB8LB-PQ#I#J", posix_create},
        {"Jmi+9w9u0E4#I#J", posix_create_name}, {"h9CcP3J0oVM#I#J", posix_join}, {"FJrT5LuUBAU#I#J", thread_exit},
    };
    for (size_t i = 0; i < sizeof(table)/sizeof(*table); ++i)
        if (!strcmp(name, table[i].nid)) return (uintptr_t)table[i].fn;
    return 0;
}

void runtime_thread_report(void) {
    r_thread_lock();
    printf("Runtime: guest threads created=%zu, exited=%zu, joined=%zu\n", g_threads_created, g_threads_exited, g_threads_joined);
    r_thread_unlock();
}
