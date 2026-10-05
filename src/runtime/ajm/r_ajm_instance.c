/* SPDX-License-Identifier: MIT
 * PS4 Ajm Codec Instance Lifecycle.
 * Single responsibility: Instance slot allocation, parameter initialization, and teardown. (~50 LOC)
 */
#include "r_ajm_types.h"

void Atrac9ReleaseHandle(void *handle);

ABI int32_t ajm_instance_create(uint32_t id, uint32_t codec, uint64_t flags, uint32_t *out) {
    if (!out || codec >= 24) return ERR_INVALID_PARAMETER;
    if (!(flags & 7)) return ERR_WRONG_REVISION;
    if (codec != 1) { fprintf(stderr, "STOP: Ajm codec %u is not implemented\n", codec); exit(21); }
    r_ajm_lock();
    Context *c = r_ajm_context(id);
    int32_t r = !c ? ERR_INVALID_CONTEXT : !c->registered[codec] ? ERR_NOT_REGISTERED : ERR_OUT_OF_RESOURCES;
    if (c && c->registered[codec]) for (uint32_t i = 1; i <= MAX_INSTANCES; ++i) if (!c->instances[i].used) {
        Instance *in = &c->instances[i];
        memset(in, 0, sizeof(*in));
        in->used = 1; in->codec = (int)codec; in->channels_hint = (int)((flags >> 3) & 15);
        in->format = (int)((flags >> 7) & 7); in->gapless_loop = (int)((flags >> 10) & 1);
        in->codec_flags = (uint32_t)(flags >> 32);
        if (in->format > FORMAT_FLOAT) { in->used = 0; r = ERR_INVALID_PARAMETER; break; }
        *out = i | (codec << 14); r = 0; break;
    }
    r_ajm_unlock();
    return r;
}

ABI int32_t ajm_instance_destroy(uint32_t id, uint32_t instance) {
    r_ajm_lock();
    Context *c = r_ajm_context(id);
    Instance *in = c ? &c->instances[instance & MAX_INSTANCES] : NULL;
    int32_t r = !c ? ERR_INVALID_CONTEXT : !(instance & MAX_INSTANCES) || !in->used ? ERR_INVALID_INSTANCE : 0;
    if (!r) { if (in->handle) Atrac9ReleaseHandle(in->handle); memset(in, 0, sizeof(*in)); }
    r_ajm_unlock();
    return r;
}

ABI uint32_t ajm_instance_codec(uint32_t instance) {
    return (instance >> 14) & 0x1f;
}
