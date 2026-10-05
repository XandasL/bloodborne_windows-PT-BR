/* SPDX-License-Identifier: MIT
 * PS4 Ajm Batch Job Chunk Builders.
 * Single responsibility: Command buffer chunk assembly. (~70 LOC)
 */
#include "r_ajm_types.h"

uint32_t r_ajm_ident(uint32_t word) { return word & 0x3f; }
uint32_t r_ajm_payload(uint32_t word) { return (word >> 6) & 0xfffff; }
uint32_t r_ajm_header(uint32_t id, uint32_t val) { return (id & 0x3f) | ((val & 0xfffff) << 6); }
int r_ajm_pcm_size(int format) { return format == FORMAT_S16 ? 2 : 4; }

static unsigned char *put_buffer(unsigned char *p, uint32_t id, const void *addr, uint64_t size) {
    ChunkBuffer c = {r_ajm_header(id, 0), (uint32_t)size, (void*)addr};
    memcpy(p, &c, sizeof(c)); return p + sizeof(c);
}
static unsigned char *put_flags(unsigned char *p, uint32_t id, uint64_t flags) {
    Chunk c = {r_ajm_header(id, (uint32_t)(flags >> 32)), (uint32_t)flags};
    memcpy(p, &c, sizeof(c)); return p + sizeof(c);
}
static void *finish_job(unsigned char *start, unsigned char *end, uint32_t instance) {
    Chunk job = {r_ajm_header(IDENT_JOB, instance), (uint32_t)(end - start - sizeof(Chunk))};
    memcpy(start, &job, sizeof(job)); return end;
}

ABI void *job_control(void *buffer, uint32_t instance, uint64_t flags, void *in, uint64_t in_size,
                      void *out, uint64_t out_size, void *ret_addr) {
    unsigned char *p = (unsigned char*)buffer + sizeof(Chunk);
    if (ret_addr) p = put_buffer(p, IDENT_RETURN_ADDRESS, ret_addr, 0);
    p = put_buffer(p, IDENT_INPUT_CONTROL, in, in_size);
    flags &= (instance == 0x80000u) ? UINT64_C(0xC0018007) : UINT64_C(0x60000000E7FF);
    p = put_flags(p, IDENT_CONTROL_FLAGS, flags);
    p = put_buffer(p, IDENT_OUTPUT_CONTROL, out, out_size);
    return finish_job(buffer, p, instance);
}

ABI void *job_run(void *buffer, uint32_t instance, uint64_t flags, void *in, uint64_t in_size,
                  void *out, uint64_t out_size, void *sideband, uint64_t side_size, void *ret_addr) {
    unsigned char *p = (unsigned char*)buffer + sizeof(Chunk);
    if (ret_addr) p = put_buffer(p, IDENT_RETURN_ADDRESS, ret_addr, 0);
    p = put_buffer(p, IDENT_INPUT_RUN, in, in_size);
    p = put_flags(p, IDENT_RUN_FLAGS, flags & UINT64_C(0xE00000001FFF));
    p = put_buffer(p, IDENT_OUTPUT_RUN, out, out_size);
    p = put_buffer(p, IDENT_OUTPUT_CONTROL, sideband, side_size);
    return finish_job(buffer, p, instance);
}

typedef struct { void *address; uint64_t size; } AjmBuffer;
ABI void *job_run_split(void *buffer, uint32_t instance, uint64_t flags, const AjmBuffer *in, uint64_t in_cnt,
                        const AjmBuffer *out, uint64_t out_cnt, void *side, uint64_t side_sz, void *ret) {
    unsigned char *p = (unsigned char*)buffer + sizeof(Chunk);
    if (ret) p = put_buffer(p, IDENT_RETURN_ADDRESS, ret, 0);
    for (uint64_t i = 0; i < in_cnt; ++i) p = put_buffer(p, IDENT_INPUT_RUN, in[i].address, in[i].size);
    p = put_flags(p, IDENT_RUN_FLAGS, flags & UINT64_C(0xE00000001FFF));
    for (uint64_t i = 0; i < out_cnt; ++i) p = put_buffer(p, IDENT_OUTPUT_RUN, out[i].address, out[i].size);
    p = put_buffer(p, IDENT_OUTPUT_CONTROL, side, side_sz);
    return finish_job(buffer, p, instance);
}

ABI void *job_inline(void *buffer, const void *data, uint64_t size, const void **address) {
    Chunk c = {r_ajm_header(IDENT_INLINE, 0), (uint32_t)((size + 7) & ~UINT64_C(7))};
    memcpy(buffer, &c, sizeof(c));
    unsigned char *p = (unsigned char*)buffer + sizeof(c);
    memcpy(p, data, size);
    if (address) *address = p;
    return p + c.size;
}
