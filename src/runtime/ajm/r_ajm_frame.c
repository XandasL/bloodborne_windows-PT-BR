/* SPDX-License-Identifier: MIT
 * PS4 Ajm ATRAC9 Superframe and Sample Decoding.
 * Single responsibility: Single ATRAC9 frame decoding and format conversion. (~70 LOC)
 */
#include "r_ajm_types.h"

int Atrac9Decode(void *handle, const unsigned char *in, int16_t *out, int *used);
int r_ajm_gapless_end(const Instance *in);
uint64_t r_ajm_output_room(const Output *o);
void r_ajm_output_write(Output *o, const unsigned char *data, uint64_t size);

uint32_t r_ajm_at9_frame(Instance *in, const unsigned char **input, uint64_t *input_size,
                         Output *out, uint32_t *samples_written, int32_t *internal) {
    int channels = in->info.channels, frame = in->info.frameSamples, used = 0;
    int16_t pcm[256 * 8];
    if (frame * channels > (int)(sizeof(pcm) / sizeof(*pcm))) return RESULT_CODEC_ERROR | RESULT_FATAL;
    int e = Atrac9Decode(in->handle, *input, pcm, &used);
    if (e) { *internal = e; return RESULT_CODEC_ERROR | RESULT_FATAL; }
    *input += used; *input_size -= (uint64_t)used; in->superframe_remain -= (uint32_t)used;
    ++g_ajm_frames_decoded;
    uint32_t skip = 0;
    if (in->gapless.skip_samples) {
        skip = in->gapless.skip_samples < (uint32_t)frame ? in->gapless.skip_samples : (uint32_t)frame;
        in->gapless.skip_samples -= (uint16_t)skip;
    }
    uint32_t samples = (uint32_t)frame - skip;
    if (in->gapless_init.total_samples && samples > in->gapless.total_samples) samples = in->gapless.total_samples;
    uint64_t room = r_ajm_output_room(out) / (uint64_t)(channels * r_ajm_pcm_size(in->format));
    if (samples > room) samples = (uint32_t)room;
    unsigned char converted[256 * 8 * 4];
    int non_interleaved = (in->codec_flags >> 8) & 1;
    for (uint32_t s = 0; s < samples; ++s) for (int c = 0; c < channels; ++c) {
        int16_t v = pcm[(skip + s) * (uint32_t)channels + (uint32_t)c];
        size_t idx = non_interleaved ? (size_t)c * samples + s : (size_t)s * (size_t)channels + (size_t)c;
        if (in->format == FORMAT_S16) memcpy(converted + idx * 2, &v, 2);
        else if (in->format == FORMAT_S32) { int32_t w = (int32_t)v << 16; memcpy(converted + idx * 4, &w, 4); }
        else { float f = (float)v / 32768.0f; memcpy(converted + idx * 4, &f, 4); }
    }
    r_ajm_output_write(out, converted, (uint64_t)samples * (uint64_t)channels * (uint64_t)r_ajm_pcm_size(in->format));
    *samples_written = samples;
    in->gapless.skipped_samples += (uint16_t)((uint32_t)frame - samples);
    if (in->gapless_init.total_samples) in->gapless.total_samples -= samples;
    if (++in->frames % (uint32_t)in->info.framesInSuperframe == 0) {
        uint64_t skip_bytes = in->superframe_remain < *input_size ? in->superframe_remain : *input_size;
        *input += skip_bytes; *input_size -= skip_bytes;
        in->superframe_remain = (uint32_t)in->info.superframeSize; in->frames = 0;
    } else if (r_ajm_gapless_end(in)) {
        while (in->frames % (uint32_t)in->info.framesInSuperframe && *input_size) {
            if (Atrac9Decode(in->handle, *input, pcm, &used)) break;
            *input += used; *input_size -= (uint64_t)used; in->superframe_remain -= (uint32_t)used; ++in->frames;
        }
        uint64_t skip_bytes = in->superframe_remain < *input_size ? in->superframe_remain : *input_size;
        *input += skip_bytes; *input_size -= skip_bytes;
        in->superframe_remain = (uint32_t)in->info.superframeSize; in->frames = 0;
    }
    return 0;
}
