/* SPDX-License-Identifier: MIT
 * PS4 Ajm Job Execution and Batch Audio Processing.
 * Single responsibility: Instance dispatch and frame execution loop. (~75 LOC)
 */
#include "r_ajm_types.h"

void r_ajm_at9_reset(Instance *in);
void r_ajm_gapless_reset(Instance *in);
int r_ajm_gapless_end(const Instance *in);
uint32_t r_ajm_next_frame_bytes(const Instance *in);
void r_ajm_parse_riff(Instance *in, const unsigned char **input, uint64_t *size);
uint32_t r_ajm_at9_frame(Instance *in, const unsigned char **input, uint64_t *input_size,
                         Output *out, uint32_t *samples_written, int32_t *internal);
uint64_t r_ajm_output_room(const Output *o);
void r_ajm_write_sideband(Instance *in, uint64_t f, unsigned char *side, unsigned char *side_end,
                         uint64_t in_start, uint64_t in_size, uint64_t out_start, uint64_t out_room, uint32_t frames);

void r_ajm_run_job(Context *ctx, uint32_t id, Job *job) {
    ++g_ajm_jobs_run;
    unsigned char *side = job->output_control.size ? (unsigned char*)job->output_control.address : NULL;
    unsigned char *side_end = side ? side + job->output_control.size : NULL;
    SidebandResult *result = NULL;
    if (side && side + sizeof(SidebandResult) <= side_end) { result = (void*)side; side += sizeof(*result); memset(result, 0, sizeof(*result)); }
    if (id == 0x80000u) { if (side) memset(side, 0, (size_t)(side_end - side)); return; }
    Instance *in = &ctx->instances[id & MAX_INSTANCES];
    if ((id & MAX_INSTANCES) == 0 || !in->used) { if (result) result->result = (int32_t)(RESULT_FATAL | RESULT_INVALID_PARAMETER); return; }
    uint64_t f = job->flags; uint32_t flags_res = 0;
    const unsigned char *ctrl = job->input_control.size ? (const unsigned char*)job->input_control.address : NULL;
    const unsigned char *ctrl_end = ctrl ? ctrl + job->input_control.size : NULL;
    if (CONTROL_RESET(f)) { in->total_samples = 0; r_ajm_gapless_reset(in); if (in->initialized) r_ajm_at9_reset(in); }
    if (ctrl && SIDEBAND_FORMAT(f) && ctrl + 24 <= ctrl_end) ctrl += 24;
    if (ctrl && SIDEBAND_GAPLESS(f) && ctrl + 8 <= ctrl_end) {
        SidebandGapless g; memcpy(&g, ctrl, 8); ctrl += 8;
        uint32_t proc = in->gapless_init.total_samples - in->gapless.total_samples;
        if (g.total_samples || !g.skip_samples) {
            if (g.total_samples >= proc) {
                int64_t diff = (int64_t)in->gapless_init.total_samples - g.total_samples;
                in->gapless_init.total_samples = g.total_samples; in->gapless.total_samples = (uint32_t)(in->gapless.total_samples - diff);
            } else flags_res |= RESULT_INVALID_PARAMETER;
        }
    }
    if (ctrl && CONTROL_INITIALIZE(f) && ctrl + 8 <= ctrl_end) { memcpy(in->config, ctrl, 4); r_ajm_at9_reset(in); }
    uint64_t in_size = 0;
    for (int i = 0; i < job->input_count; ++i) in_size += job->inputs[i].size;
    unsigned char *joined = NULL; const unsigned char *input = NULL;
    if (job->input_count == 1) input = (const unsigned char*)job->inputs[0].address;
    else if (job->input_count > 1) {
        joined = (unsigned char*)malloc(in_size ? in_size : 1); uint64_t at = 0;
        for (int i = 0; i < job->input_count; ++i) { memcpy(joined + at, job->inputs[i].address, job->inputs[i].size); at += job->inputs[i].size; }
        input = joined;
    }
    Output out = {job->outputs, job->output_count, 0, 0};
    uint64_t in_start = in_size, out_start = r_ajm_output_room(&out); uint32_t frames = 0; int32_t internal = 0;
    if (in_size) for (;;) {
        if (in->gapless_loop && r_ajm_gapless_end(in)) { r_ajm_gapless_reset(in); in->total_samples = 0; }
        if ((in->codec_flags & 1) && in_size >= 4 && !memcmp(input, "RIFF", 4)) { r_ajm_parse_riff(in, &input, &in_size); in->total_samples = 0; }
        if (!in->initialized) { flags_res |= RESULT_NOT_INITIALIZED; break; }
        if (!r_ajm_gapless_end(in) && r_ajm_output_room(&out) < r_ajm_next_frame_bytes(in)) flags_res |= RESULT_NOT_ENOUGH_ROOM;
        if (in_size < in->superframe_remain) flags_res |= RESULT_PARTIAL_INPUT;
        if (flags_res) break;
        uint32_t written = 0; uint32_t r = r_ajm_at9_frame(in, &input, &in_size, &out, &written, &internal);
        in->total_samples += written; ++frames;
        if (r) { flags_res |= r; break; }
        if (!RUN_MULTIPLE_FRAMES(f)) break;
    }
    free(joined);
    if (result) { result->result = (int32_t)flags_res; result->internal_result = internal; }
    r_ajm_write_sideband(in, f, side, side_end, in_start, in_size, out_start, r_ajm_output_room(&out), frames);
}
