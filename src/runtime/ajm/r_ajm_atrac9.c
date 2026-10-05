/* SPDX-License-Identifier: MIT
 * PS4 Ajm ATRAC9 Decoding and RIFF Container Parser.
 * Single responsibility: Frame decoding and safe RIFF demuxing. (~72 LOC)
 */
#include "r_ajm_types.h"

void *Atrac9GetHandle(void);
void Atrac9ReleaseHandle(void *handle);
int Atrac9InitDecoder(void *handle, const unsigned char *config);
int Atrac9GetCodecInfo(void *handle, void *info);
int Atrac9Decode(void *handle, const unsigned char *in, int16_t *out, int *used);

uint64_t r_ajm_output_room(const Output *o);
void r_ajm_output_write(Output *o, const unsigned char *data, uint64_t size);

void r_ajm_at9_reset(Instance *in) {
    if (in->handle) Atrac9ReleaseHandle(in->handle);
    in->handle = Atrac9GetHandle();
    in->initialized = in->handle && (Atrac9InitDecoder(in->handle, in->config) == 0);
    if (in->initialized) Atrac9GetCodecInfo(in->handle, &in->info);
    in->frames = 0; in->superframe_remain = (uint32_t)in->info.superframeSize;
}

void r_ajm_gapless_reset(Instance *in) {
    in->gapless.total_samples = in->gapless_init.total_samples;
    in->gapless.skip_samples = in->gapless_init.skip_samples;
    in->gapless.skipped_samples = 0;
}

int r_ajm_gapless_end(const Instance *in) {
    return in->gapless_init.total_samples && !in->gapless.total_samples;
}

uint32_t r_ajm_next_frame_bytes(const Instance *in) {
    uint32_t frame = (uint32_t)in->info.frameSamples;
    uint32_t skip = in->gapless.skip_samples < frame ? in->gapless.skip_samples : frame;
    uint32_t samples = frame - skip;
    if (in->gapless_init.total_samples && in->gapless.total_samples < samples) samples = in->gapless.total_samples;
    return samples * (uint32_t)in->info.channels * (uint32_t)r_ajm_pcm_size(in->format);
}

void r_ajm_parse_riff(Instance *in, const unsigned char **input, uint64_t *size) {
    if (!input || !*input || *size < 12) return;
    const unsigned char *p = *input, *end = p + *size;
    if (memcmp(p, "RIFF", 4) != 0) return;
    p += 12;
    while (p + 8 <= end) {
        uint32_t tag, length; memcpy(&tag, p, 4); memcpy(&length, p + 4, 4); p += 8;
        if (tag == 0x61746164) break;
        if (tag == 0x20746d66 && length >= 52 && p + 52 <= end) {
            memcpy(in->config, p + 44, 4); r_ajm_at9_reset(in);
        } else if (tag == 0x74636166 && length >= 12 && p + 12 <= end) {
            uint32_t total, delay; memcpy(&total, p, 4); memcpy(&delay, p + 4, 4);
            in->gapless_init.total_samples = total; in->gapless_init.skip_samples = (uint16_t)delay;
            r_ajm_gapless_reset(in);
        }
        if (length > (size_t)(end - p)) { p = end; break; }
        p += length;
    }
    if (p > end) p = end;
    *size -= (uint64_t)(p - *input); *input = p;
}
