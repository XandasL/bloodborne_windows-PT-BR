/* SPDX-License-Identifier: MIT
 * PS4 Ajm NID Exports Table and Reporting.
 * Single responsibility: Export table and diagnostic stats. (~55 LOC)
 */
#include "r_ajm_types.h"

ABI int32_t ajm_initialize(int64_t reserved, uint32_t *out);
ABI int32_t ajm_finalize(uint32_t id);
ABI int32_t ajm_module_register(uint32_t id, uint32_t codec, int64_t reserved);
ABI int32_t ajm_module_unregister(uint32_t id, uint32_t codec);
ABI int32_t ajm_instance_create(uint32_t id, uint32_t codec, uint64_t flags, uint32_t *out);
ABI int32_t ajm_instance_destroy(uint32_t id, uint32_t instance);
ABI uint32_t ajm_instance_codec(uint32_t instance);
ABI void *job_control(void *buf, uint32_t inst, uint64_t fl, void *in, uint64_t isz, void *out, uint64_t osz, void *ra);
ABI void *job_run(void *buf, uint32_t inst, uint64_t fl, void *in, uint64_t isz, void *out, uint64_t osz, void *sb, uint64_t sbsz, void *ra);
ABI void *job_run_split(void *buf, uint32_t inst, uint64_t fl, const void *in, uint64_t icnt, const void *out, uint64_t ocnt, void *sb, uint64_t sbsz, void *ra);
ABI void *job_inline(void *buf, const void *data, uint64_t sz, const void **addr);
ABI int32_t ajm_batch_start(uint32_t id, unsigned char *buf, uint32_t sz, int prio, BatchError *err, uint32_t *out);
ABI int32_t ajm_batch_wait(uint32_t id, uint32_t batch, uint32_t timeout, BatchError *error);
ABI int32_t ajm_batch_cancel(uint32_t id, uint32_t batch);
ABI int32_t ajm_error_dump(void);
ABI int32_t ajm_memory_register(uint32_t id, void *p, uint64_t pages);

static const RuntimeExport exports[] = {
    {"sceAjmInitialize", (void*)ajm_initialize}, {"sceAjmFinalize", (void*)ajm_finalize},
    {"sceAjmModuleRegister", (void*)ajm_module_register}, {"sceAjmModuleUnregister", (void*)ajm_module_unregister},
    {"sceAjmInstanceCreate", (void*)ajm_instance_create}, {"sceAjmInstanceDestroy", (void*)ajm_instance_destroy},
    {"sceAjmInstanceCodecType", (void*)ajm_instance_codec},
    {"sceAjmBatchJobControlBufferRa", (void*)job_control}, {"sceAjmBatchJobRunBufferRa", (void*)job_run},
    {"sceAjmBatchJobRunSplitBufferRa", (void*)job_run_split}, {"sceAjmBatchJobInlineBuffer", (void*)job_inline},
    {"sceAjmBatchStartBuffer", (void*)ajm_batch_start}, {"sceAjmBatchWait", (void*)ajm_batch_wait},
    {"sceAjmBatchCancel", (void*)ajm_batch_cancel}, {"sceAjmBatchErrorDump", (void*)ajm_error_dump},
    {"sceAjmMemoryRegister", (void*)ajm_memory_register},
};

uintptr_t runtime_ajm_resolve(const char *name) {
    return RUNTIME_LOOKUP(exports, name);
}

void runtime_ajm_report(void) {
    printf("Runtime: Ajm batches=%zu, jobs=%zu, ATRAC9 frames decoded=%zu\n",
           g_ajm_batches_run, g_ajm_jobs_run, g_ajm_frames_decoded);
}
