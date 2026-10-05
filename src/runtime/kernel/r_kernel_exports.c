/* SPDX-License-Identifier: MIT
 * PS4 Kernel Services NID Exports Table.
 * Single responsibility: Export table and symbol resolution. (~65 LOC)
 */
#include "r_kernel_types.h"

ABI int32_t kernel_clock_gettime(uint32_t id, GuestTimespec *ts);
ABI int32_t posix_clock_gettime(uint32_t id, GuestTimespec *ts);
ABI int32_t posix_clock_getres(uint32_t id, GuestTimespec *ts);
ABI uint64_t process_time(void);
ABI uint64_t process_time_counter(void);
ABI uint64_t process_time_frequency(void);
ABI uint64_t read_tsc(void);
ABI uint64_t tsc_frequency(void);
ABI int32_t kernel_usleep(uint32_t usec);
ABI int32_t posix_usleep(uint32_t usec);
ABI uint32_t posix_sleep(uint32_t seconds);
ABI int32_t posix_nanosleep(const GuestTimespec *rq, GuestTimespec *rem);
ABI int32_t kernel_nanosleep(const GuestTimespec *rq, GuestTimespec *rem);
ABI int32_t kernel_gettimezone(GuestTimezone *tz);
ABI int32_t posix_gettimeofday(GuestTimeval *tv, GuestTimezone *tz);
ABI int32_t kernel_gettimeofday(GuestTimeval *tv);
ABI int64_t posix_time(int64_t *out);
ABI int32_t get_pagesize(void);
ABI int32_t get_pid(void);
ABI int32_t yield(void);
ABI __attribute__((noreturn)) void hard_exit(int status);
ABI __attribute__((noreturn)) void raise_exception(uint32_t code, uint64_t argument);
ABI int32_t print_backtrace(void);
ABI uintptr_t guest_signal(int sig, uintptr_t handler);
ABI int32_t guest_sigprocmask(int how, const GuestSigset *set, GuestSigset *old);
ABI int32_t guest_sigfillset(GuestSigset *set);
ABI int32_t guest_sigemptyset(GuestSigset *set);
ABI int32_t guest_getrusage(int who, GuestRusage *out);
ABI int32_t guest_sysctl(const int32_t *name, uint32_t namelen, void *old, uint64_t *oldlen, const void *new_val, uint64_t newlen);
ABI int32_t thread_once(int32_t *once, void (ABI *routine)(void));
ABI int32_t posix_once(int32_t *once, void (ABI *routine)(void));
ABI int32_t key_create(uint32_t *key, KeyDestructor destructor);
ABI int32_t key_delete(uint32_t key);
ABI int32_t key_set(uint32_t key, void *value);
ABI void *key_get(uint32_t key);
ABI int32_t posix_key_create(uint32_t *k, KeyDestructor d);
ABI int32_t posix_key_delete(uint32_t k);
ABI int32_t posix_key_set(uint32_t k, void *v);

static const RuntimeExport exports[] = {
    {"sceKernelClockGettime", (void*)kernel_clock_gettime}, {"clock_gettime", (void*)posix_clock_gettime},
    {"clock_getres", (void*)posix_clock_getres},
    {"sceKernelGetProcessTime", (void*)process_time}, {"sceKernelGetProcessTimeCounter", (void*)process_time_counter},
    {"sceKernelGetProcessTimeCounterFrequency", (void*)process_time_frequency},
    {"sceKernelReadTsc", (void*)read_tsc}, {"sceKernelGetTscFrequency", (void*)tsc_frequency},
    {"sceKernelUsleep", (void*)kernel_usleep}, {"usleep", (void*)posix_usleep}, {"sleep", (void*)posix_sleep},
    {"nanosleep", (void*)posix_nanosleep}, {"sceKernelNanosleep", (void*)kernel_nanosleep},
    {"sceKernelGettimezone", (void*)kernel_gettimezone}, {"gettimeofday", (void*)posix_gettimeofday},
    {"sceKernelGettimeofday", (void*)kernel_gettimeofday}, {"time", (void*)posix_time},
    {"getpagesize", (void*)get_pagesize}, {"getpid", (void*)get_pid}, {"sched_yield", (void*)yield},
    {"_exit", (void*)hard_exit}, {"sceKernelDebugRaiseException", (void*)raise_exception},
    {"sceKernelDebugRaiseExceptionOnReleaseMode", (void*)raise_exception},
    {"sceKernelPrintBacktraceWithModuleInfo", (void*)print_backtrace},
    {"signal", (void*)guest_signal}, {"sigprocmask", (void*)guest_sigprocmask}, {"_sigprocmask", (void*)guest_sigprocmask},
    {"sigfillset", (void*)guest_sigfillset}, {"sigemptyset", (void*)guest_sigemptyset},
    {"getrusage", (void*)guest_getrusage}, {"sysctl", (void*)guest_sysctl},
    {"scePthreadOnce", (void*)thread_once}, {"pthread_once", (void*)posix_once},
    {"scePthreadKeyCreate", (void*)key_create}, {"scePthreadKeyDelete", (void*)key_delete},
    {"scePthreadSetspecific", (void*)key_set}, {"scePthreadGetspecific", (void*)key_get},
    {"pthread_key_create", (void*)posix_key_create}, {"pthread_key_delete", (void*)posix_key_delete},
    {"pthread_setspecific", (void*)posix_key_set}, {"pthread_getspecific", (void*)key_get},
};

uintptr_t runtime_kernel_resolve(const char *name) {
    return RUNTIME_LOOKUP(exports, name);
}
