/* SPDX-License-Identifier: MIT
 * PS4 Trophy Service Offline Implementation.
 * Single responsibility: Local trophy unlocking and metadata queries. (~50 LOC)
 */
#include "r_serv_types.h"

ABI int32_t trophy_context(int32_t *ctx, int32_t user, uint32_t label, uint64_t options) {
    (void)user; (void)label; (void)options;
    if (!ctx) return SERV_TROPHY_INVALID;
    *ctx = serv_new_id();
    return 0;
}

ABI int32_t trophy_handle(int32_t *handle) {
    if (!handle) return SERV_TROPHY_INVALID;
    *handle = serv_new_id();
    return 0;
}

ABI int32_t trophy_register(int32_t ctx, int32_t handle, uint64_t options) {
    (void)ctx; (void)handle; (void)options;
    return 0;
}

ABI int32_t trophy_unlock(int32_t ctx, int32_t handle, int32_t id, int32_t *platinum) {
    (void)ctx; (void)handle;
    printf("Runtime: trophy %d unlocked\n", id);
    if (platinum) *platinum = -1;
    return 0;
}

ABI int32_t trophy_game_info(int32_t ctx, int32_t handle, void *details, void *data) {
    (void)ctx; (void)handle;
    if (details) {
        uint64_t size; memcpy(&size, details, 8);
        memset((char *)details + 8, 0, (size > 8 && size < 4096) ? size - 8 : 0);
    }
    if (data) {
        uint64_t size; memcpy(&size, data, 8);
        memset((char *)data + 8, 0, (size > 8 && size < 4096) ? size - 8 : 0);
    }
    return 0;
}

ABI int32_t trophy_info(int32_t ctx, int32_t handle, int32_t id, void *details, void *data) {
    (void)id;
    return trophy_game_info(ctx, handle, details, data);
}
