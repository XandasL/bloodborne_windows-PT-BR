/* SPDX-License-Identifier: MIT
 * PS4 Ajm Job Output Sideband Serialization.
 * Single responsibility: Sideband format, stream, and gapless reporting. (~45 LOC)
 */
#include "r_ajm_types.h"

static uint32_t channel_mask(int ch) {
    switch (ch) { case 1: return 4; case 2: return 3; case 4: return 0x33; case 6: return 0x60f; case 8: return 0x63f; default: return 0; }
}

void r_ajm_write_sideband(Instance *in, uint64_t f, unsigned char *side, unsigned char *side_end,
                         uint64_t in_start, uint64_t in_size, uint64_t out_start, uint64_t out_room, uint32_t frames) {
    if (!side) return;
    if (SIDEBAND_STREAM(f) && side + 16 <= side_end) {
        SidebandStream s = {(int32_t)(in_start - in_size), (int32_t)(out_start - out_room), in->total_samples};
        memcpy(side, &s, 16); side += 16;
    }
    if (SIDEBAND_FORMAT(f) && side + 24 <= side_end) {
        SidebandFormat s = {(uint32_t)in->info.channels, channel_mask(in->info.channels), (uint32_t)in->info.samplingRate,
                            (uint32_t)in->format, 0, 0};
        if (in->info.framesInSuperframe && in->info.frameSamples)
            s.bitrate = (uint32_t)((uint64_t)in->info.samplingRate * (uint64_t)in->info.superframeSize * 8 /
                                  ((uint64_t)in->info.framesInSuperframe * (uint64_t)in->info.frameSamples));
        memcpy(side, &s, 24); side += 24;
    }
    if (SIDEBAND_GAPLESS(f) && side + 8 <= side_end) { memcpy(side, &in->gapless, 8); side += 8; }
    if (RUN_MULTIPLE_FRAMES(f) && side + 8 <= side_end) { uint32_t m[2] = {frames, 0}; memcpy(side, m, 8); side += 8; }
    if (RUN_CODEC_INFO(f) && side + 16 <= side_end) {
        At9Info info = {(uint32_t)in->info.superframeSize, (uint32_t)in->info.framesInSuperframe, in->superframe_remain, (uint32_t)in->info.frameSamples};
        memcpy(side, &info, 16);
    }
}
