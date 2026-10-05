/* SPDX-License-Identifier: MIT
 * PS4 Signal State and Mask Bookkeeping.
 * Single responsibility: Guest signal masks and handler tables. (~55 LOC)
 */
#include "r_kernel_types.h"

static _Thread_local GuestSigset signal_mask;
static uintptr_t handlers[128];

ABI uintptr_t guest_signal(int sig, uintptr_t handler) {
    if (sig <= 0 || sig >= 128) { *runtime_errno() = 22; return (uintptr_t)-1; }
    uintptr_t old = handlers[sig];
    handlers[sig] = handler;
    printf("Runtime: guest signal(%d) handler recorded\n", sig);
    return old;
}

ABI int32_t guest_sigprocmask(int how, const GuestSigset *set, GuestSigset *old) {
    if (old) *old = signal_mask;
    if (!set) return 0;
    for (int i = 0; i < 4; ++i) {
        if (how == 1) signal_mask.bits[i] |= set->bits[i];
        else if (how == 2) signal_mask.bits[i] &= ~set->bits[i];
        else if (how == 3) signal_mask.bits[i] = set->bits[i];
        else return r_kernel_fail_posix(22);
    }
    return 0;
}

ABI int32_t guest_sigfillset(GuestSigset *set) {
    if (!set) return r_kernel_fail_posix(22);
    memset(set, 0xff, sizeof(*set));
    return 0;
}

ABI int32_t guest_sigemptyset(GuestSigset *set) {
    if (!set) return r_kernel_fail_posix(22);
    memset(set, 0, sizeof(*set));
    return 0;
}
