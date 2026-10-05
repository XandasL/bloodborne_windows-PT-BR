/* SPDX-License-Identifier: MIT
 * PS4 Network Platform (NP/PSN) Offline Emulation.
 * Single responsibility: Emulated offline state, callbacks, and auth tokens. (~55 LOC)
 */
#include "r_serv_types.h"

ABI int32_t np_state(int32_t user, int32_t *state) {
    if (!state) return SERV_NP_INVALID_ARGUMENT;
    (void)user; *state = 1;
    return 0;
}

ABI int32_t np_signed_out(void) { return SERV_NP_SIGNED_OUT; }
ABI int32_t np_register(void *cb, void *arg) { (void)cb; (void)arg; return serv_new_id(); }
ABI void np_register_void(void *cb, void *arg) { (void)cb; (void)arg; }
ABI int32_t np_request(const void *param) { (void)param; return serv_new_id(); }
ABI int32_t np_request_ctx(int32_t ctx, const void *param) { (void)ctx; (void)param; return serv_new_id(); }
ABI int32_t np_poll(int32_t request, int32_t *result) { (void)request; if (result) *result = SERV_NP_SIGNED_OUT; return 0; }

ABI int32_t np_compare(const void *a, const void *b) {
    if (!a || !b) return SERV_NP_INVALID_ARGUMENT;
    return memcmp(a, b, 16) ? (int32_t)0x80550609 : 0;
}
