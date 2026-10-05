/* SPDX-License-Identifier: MIT
 * PS4 Ajm Context Lifecycle and Codec Registration.
 * Single responsibility: Decoder context allocation and module registration. (~55 LOC)
 */
#include "r_ajm_types.h"

Context *g_ajm_contexts[MAX_CONTEXTS + 1];
size_t g_ajm_jobs_run = 0, g_ajm_frames_decoded = 0, g_ajm_batches_run = 0;
static BbMutex *ajm_lock;

void Atrac9ReleaseHandle(void *handle);

static void ensure_lock(void) {
    if (!ajm_lock) {
        BbMutex *m = bb_platform_mutex_create(BB_MUTEX_TYPE_RECURSIVE);
        if (!__sync_bool_compare_and_swap(&ajm_lock, NULL, m)) bb_platform_mutex_destroy(m);
    }
}
void r_ajm_lock(void) { ensure_lock(); bb_platform_mutex_lock(ajm_lock); }
void r_ajm_unlock(void) { if (ajm_lock) bb_platform_mutex_unlock(ajm_lock); }
Context *r_ajm_context(uint32_t id) { return id && id <= MAX_CONTEXTS ? g_ajm_contexts[id] : NULL; }

ABI int32_t ajm_initialize(int64_t reserved, uint32_t *out) {
    if (!out || reserved) return ERR_INVALID_PARAMETER;
    r_ajm_lock();
    for (uint32_t i = 1; i <= MAX_CONTEXTS; ++i) if (!g_ajm_contexts[i]) {
        g_ajm_contexts[i] = (Context*)calloc(1, sizeof(Context));
        r_ajm_unlock();
        if (!g_ajm_contexts[i]) return ERR_OUT_OF_RESOURCES;
        *out = i; puts("Runtime: Ajm context initialized (ATRAC9 via LibAtrac9)");
        return 0;
    }
    r_ajm_unlock();
    return ERR_OUT_OF_RESOURCES;
}

ABI int32_t ajm_finalize(uint32_t id) {
    r_ajm_lock();
    Context *c = r_ajm_context(id);
    if (!c) { r_ajm_unlock(); return ERR_INVALID_CONTEXT; }
    for (int i = 0; i <= MAX_INSTANCES; ++i) if (c->instances[i].handle) Atrac9ReleaseHandle(c->instances[i].handle);
    free(c); g_ajm_contexts[id] = NULL;
    r_ajm_unlock();
    return 0;
}

ABI int32_t ajm_module_register(uint32_t id, uint32_t codec, int64_t reserved) {
    if (reserved || codec >= 24) return ERR_INVALID_PARAMETER;
    r_ajm_lock();
    Context *c = r_ajm_context(id);
    int32_t r = !c ? ERR_INVALID_CONTEXT : c->registered[codec] ? ERR_ALREADY_REGISTERED : 0;
    if (!r) c->registered[codec] = 1;
    r_ajm_unlock();
    return r;
}

ABI int32_t ajm_module_unregister(uint32_t id, uint32_t codec) {
    r_ajm_lock();
    Context *c = r_ajm_context(id);
    int32_t r = !c ? ERR_INVALID_CONTEXT : codec >= 24 ? ERR_INVALID_PARAMETER : !c->registered[codec] ? ERR_NOT_REGISTERED : 0;
    if (!r) c->registered[codec] = 0;
    r_ajm_unlock();
    return r;
}
