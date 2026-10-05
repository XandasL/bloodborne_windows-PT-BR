/* SPDX-License-Identifier: MIT
 * PS4 Http & Ssl Offline Provider.
 * Single responsibility: Emulated HTTP handles and offline error signaling. (~50 LOC)
 */
#include "r_serv_types.h"

ABI int32_t lib_init_id(void) { return serv_new_id(); }
ABI int32_t ok_void(void) { return 0; }
ABI int32_t http_fail(void) { return SERV_HTTP_NETWORK; }
ABI int32_t http_object(void) { return serv_new_id(); }

ABI int32_t http_epoll(int32_t ctx, void **handle) {
    (void)ctx;
    if (!handle) return (int32_t)0x80431077;
    *handle = (void *)(uintptr_t)(0x100 + serv_new_id());
    return 0;
}

ABI int32_t http_wait(void *handle, void *events, int32_t max, int64_t timeout) {
    (void)handle; (void)events; (void)max;
    if (timeout > 0) bb_platform_sleep_ns((uint64_t)timeout * 1000ULL);
    return 0;
}
