/* SPDX-License-Identifier: MIT
 * PS4 Core Symbol Dispatch Engine.
 * Single responsibility: Delegating NID lookups across all subsystem resolvers. (~70 LOC)
 */
#include "r_core_types.h"
#include "gpu/bbgpu.h"

ABI void guest_init_env(void);
ABI int guest_atexit(GuestCallback cb);
ABI int guest_cxa_atexit(void (ABI *cb)(void *), void *arg, void *dso);
ABI void guest_finalize(void *dso);
ABI int guard_acquire(uint64_t *guard);
ABI void guard_release(uint64_t *guard);
ABI void guard_abort(uint64_t *guard);
ABI __attribute__((noreturn)) void stack_fail(void);
ABI void *guest_memset(void *dst, int value, size_t size);
ABI void *guest_memcpy(void *dst, const void *src, size_t size);
ABI void *guest_memmove(void *dst, const void *src, size_t size);
ABI int guest_memcmp(const void *a, const void *b, size_t size);
ABI size_t guest_strlen(const char *text);
ABI void *guest_tls_get_addr(const uint64_t *index);
ABI void *guest_procparam(void);
ABI void guest_set_heap_api(void **api);

static ABI __attribute__((noreturn)) void guest_libc_exit(int status) {
    printf("Runtime: guest requested exit(%d)\n", status);
    runtime_finalize(NULL);
    runtime_report();
    exit(status);
}

uintptr_t runtime_resolve(const char *name, int is_data) {
    if (!(g_runtime_capabilities & 1)) return 0;
    if (is_data) {
        static int32_t need_libc_internal = 1;
        if (!strcmp(name, "f7uOxY9mM1U#p#J")) return (uintptr_t)&g_runtime_stack_canary;
        if (!strcmp(name, "ZT4ODD2Ts9o#libSceLibcInternal")) return (uintptr_t)&need_libc_internal;
        return 0;
    }
    if (!strcmp(name, "bzQExy189ZI#q#q")) return (uintptr_t)guest_init_env;
    if (!strcmp(name, "8G2LB+A3rzg#q#q")) return (uintptr_t)guest_atexit;
    if (!strcmp(name, "tsvEmnenz48#q#q")) return (uintptr_t)guest_cxa_atexit;
    if (!strcmp(name, "uMei1W9uyNo#q#q")) return (uintptr_t)guest_libc_exit;
    if (!strcmp(name, "H2e8t5ScQGc#q#q")) return (uintptr_t)guest_finalize;
    if (!strcmp(name, "3GPpjQdAMTw#q#q")) return (uintptr_t)guard_acquire;
    if (!strcmp(name, "9rAeANT2tyE#q#q")) return (uintptr_t)guard_release;
    if (!strcmp(name, "2emaaluWzUw#q#q")) return (uintptr_t)guard_abort;
    if (!strcmp(name, "Ou3iL1abvng#p#J")) return (uintptr_t)stack_fail;
    if (!strcmp(name, "8zTFvBIAIN8#q#q")) return (uintptr_t)guest_memset;
    if (!strcmp(name, "Q3VBxCXhUHs#q#q")) return (uintptr_t)guest_memcpy;
    if (!strcmp(name, "+P6FRGH4LfA#q#q")) return (uintptr_t)guest_memmove;
    if (!strcmp(name, "DfivPArhucg#q#q")) return (uintptr_t)guest_memcmp;
    if (!strcmp(name, "j4ViWNHEgww#q#q")) return (uintptr_t)guest_strlen;
    if (!strcmp(name, "vNe1w4diLCs#p#J")) return (uintptr_t)guest_tls_get_addr;
    if (!strcmp(name, "959qrazPIrg#p#J")) return (uintptr_t)guest_procparam;
    if (!strcmp(name, "p5EcQeEeJAE#p#J")) return (uintptr_t)guest_set_heap_api;
    uintptr_t res = 0;
    if ((res = runtime_mutex_resolve(name))) return res;
    if ((res = runtime_thread_resolve(name))) return res;
    if ((res = runtime_content_resolve(name))) return res;
    if ((res = runtime_time_resolve(name))) return res;
    if ((res = runtime_sema_resolve(name))) return res;
    if ((res = runtime_rwlock_resolve(name))) return res;
    if ((res = runtime_memory_resolve(name))) return res;
    if ((res = runtime_kernel_resolve(name))) return res;
    if ((res = runtime_services_resolve(name))) return res;
    if ((res = runtime_ajm_resolve(name))) return res;
    if ((res = runtime_audio_resolve(name))) return res;
    if ((res = runtime_pad_resolve(name))) return res;
    if ((res = runtime_rtc_resolve(name))) return res;
    if ((res = runtime_savedata_resolve(name))) return res;
    if ((res = runtime_file_resolve(name))) return res;
    return bbgpu_resolve(name);
}
