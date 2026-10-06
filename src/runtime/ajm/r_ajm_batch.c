/* SPDX-License-Identifier: MIT
 * PS4 Ajm Batch Job Execution and Completion.
 * Single responsibility: Batch queue submission, synchronous wait, and cancellation. (~65 LOC)
 */
#include "r_ajm_types.h"

Batch g_ajm_batches[MAX_BATCHES];

int r_ajm_parse_job(const unsigned char *p, uint32_t size, Job *job);
void r_ajm_run_job(Context *ctx, uint32_t id, Job *job);

ABI int32_t ajm_batch_start(uint32_t id, unsigned char *buffer, uint32_t size, int priority,
                             BatchError *error, uint32_t *out) {
    (void)priority;
    if (size & 7) return ERR_MALFORMED_BATCH;
    r_ajm_lock();
    Context *c = r_ajm_context(id);
    if (!c) { r_ajm_unlock(); return ERR_INVALID_CONTEXT; }
    int slot = -1;
    for (int i = 0; i < MAX_BATCHES; ++i) if (!g_ajm_batches[i].used) { slot = i; break; }
    if (slot < 0) { r_ajm_unlock(); return ERR_OUT_OF_RESOURCES; }
    for (unsigned char *p = buffer, *end = buffer + size; p < end;) {
        Chunk chunk; memcpy(&chunk, p, 8);
        unsigned char *body = p + 8;
        p = body + chunk.size;
        if (r_ajm_ident(chunk.word) == IDENT_INLINE) continue;
        Job job;
        if (r_ajm_ident(chunk.word) != IDENT_JOB || p > end || r_ajm_parse_job(body, chunk.size, &job)) {
            if (error) { error->error_code = ERR_MALFORMED_BATCH; error->job_address = body - 8;
                         error->command_offset = (uint32_t)(body - 8 - buffer); error->job_return_address = NULL; }
            r_ajm_unlock(); return ERR_MALFORMED_BATCH;
        }
        r_ajm_run_job(c, r_ajm_payload(chunk.word), &job);
    }
    g_ajm_batches[slot] = (Batch){1, (int)id, 0};
    ++g_ajm_batches_run;
    *out = (uint32_t)slot + 1;
    r_ajm_unlock();
    return 0;
}

ABI int32_t ajm_batch_wait(uint32_t id, uint32_t batch, uint32_t timeout, BatchError *error) {
    (void)timeout; (void)error;
    r_ajm_lock();
    int32_t r = !r_ajm_context(id) ? ERR_INVALID_CONTEXT : !batch || batch > MAX_BATCHES || !g_ajm_batches[batch - 1].used ? ERR_INVALID_BATCH : 0;
    if (!r) { if (g_ajm_batches[batch - 1].canceled) r = ERR_CANCELLED; g_ajm_batches[batch - 1].used = 0; }
    r_ajm_unlock();
    return r;
}

ABI int32_t ajm_batch_cancel(uint32_t id, uint32_t batch) {
    r_ajm_lock();
    int32_t r = !r_ajm_context(id) ? ERR_INVALID_CONTEXT : !batch || batch > MAX_BATCHES || !g_ajm_batches[batch - 1].used ? ERR_INVALID_BATCH : 0;
    r_ajm_unlock();
    return r;
}

ABI int32_t ajm_error_dump(void) { return 0; }
ABI int32_t ajm_memory_register(uint32_t id, void *p, uint64_t pages) { (void)p; (void)pages; return r_ajm_context(id) ? 0 : ERR_INVALID_CONTEXT; }
