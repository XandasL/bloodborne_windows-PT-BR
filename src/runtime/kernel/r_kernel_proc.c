/* SPDX-License-Identifier: MIT
 * PS4 Process and Hardware System Control.
 * Single responsibility: Process identity, exits, sysctl, and rusage. (~75 LOC)
 */
#include "r_kernel_types.h"
#include "platform/threads.h"

ABI int32_t get_pagesize(void) { return PAGE; }
ABI int32_t get_pid(void) { return 1000; }
ABI int32_t yield(void) { bb_platform_thread_yield(); return 0; }

ABI __attribute__((noreturn)) void hard_exit(int status) {
    printf("Runtime: guest requested _exit(%d)\n", status);
    runtime_report();
    exit(status);
}

ABI __attribute__((noreturn)) void raise_exception(uint32_t code, uint64_t arg) {
    fprintf(stderr, "STOP: guest debug exception code=0x%x arg=0x%" PRIx64 "\n", code, arg);
    runtime_report();
    exit(23);
}

ABI int32_t print_backtrace(void) {
    fputs("Runtime: guest requested a backtrace (not available)\n", stderr);
    return 0;
}

ABI int32_t guest_getrusage(int who, GuestRusage *out) {
    if (!out || (who != 0 && who != 1)) return r_kernel_fail_posix(22);
    memset(out, 0, sizeof(*out));
    uint64_t us = runtime_process_time_us();
    out->utime = (GuestTimeval){(int64_t)(us / 1000000), (int64_t)(us % 1000000)};
    return 0;
}

ABI int32_t guest_sysctl(const int32_t *name, uint32_t namelen, void *old,
                         uint64_t *oldlen, const void *new_val, uint64_t newlen) {
    (void)newlen;
    if (!name || namelen < 2 || new_val) return r_kernel_fail_posix(22);
    if (name[0] == 1 && name[1] == 37) { /* kern.arandom */
        if (!old || !oldlen) return r_kernel_fail_posix(22);
        uint8_t *p = (uint8_t*)old;
        for (size_t i = 0; i < *oldlen; ++i) p[i] = (uint8_t)rand();
        return 0;
    }
    if (name[0] == 6 && (name[1] == 7 || name[1] == 3)) { /* hw.pagesize / hw.ncpu */
        if (!oldlen) return r_kernel_fail_posix(22);
        int32_t val = (name[1] == 7) ? PAGE : 7;
        if (old) {
            if (*oldlen < 4) return r_kernel_fail_posix(12);
            memcpy(old, &val, 4);
        }
        *oldlen = 4;
        return 0;
    }
    fprintf(stderr, "STOP: unsupported sysctl mib: %d %d\n", name[0], name[1]);
    exit(21);
}
