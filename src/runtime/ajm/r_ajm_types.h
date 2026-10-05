/* SPDX-License-Identifier: MIT
 * PS4 Ajm Audio Decoder Runtime Types.
 * Single responsibility: Chunk layouts, sideband records, and instance state. (~68 LOC)
 */
#ifndef R_AJM_TYPES_H
#define R_AJM_TYPES_H
#include "runtime.h"
#include "platform/bb_common.h"
#include "platform/sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#define ERR_INVALID_CONTEXT ((int32_t)0x80930002)
#define ERR_INVALID_INSTANCE ((int32_t)0x80930003)
#define ERR_INVALID_BATCH ((int32_t)0x80930004)
#define ERR_INVALID_PARAMETER ((int32_t)0x80930005)
#define ERR_OUT_OF_RESOURCES ((int32_t)0x80930007)
#define ERR_ALREADY_REGISTERED ((int32_t)0x80930009)
#define ERR_NOT_REGISTERED ((int32_t)0x8093000A)
#define ERR_WRONG_REVISION ((int32_t)0x8093000B)
#define ERR_MALFORMED_BATCH ((int32_t)0x80930011)
#define ERR_CANCELLED ((int32_t)0x80930017)
#define RESULT_NOT_INITIALIZED 0x1
#define RESULT_INVALID_PARAMETER 0x4
#define RESULT_PARTIAL_INPUT 0x8
#define RESULT_NOT_ENOUGH_ROOM 0x10
#define RESULT_CODEC_ERROR 0x40000000u
#define RESULT_FATAL 0x80000000u
#define MAX_INSTANCES 0x3fff
#define MAX_CONTEXTS 8
#define MAX_BATCHES 256
enum { IDENT_JOB = 0, IDENT_INPUT_RUN = 1, IDENT_INPUT_CONTROL = 2, IDENT_CONTROL_FLAGS = 3, IDENT_RUN_FLAGS = 4,
       IDENT_RETURN_ADDRESS = 6, IDENT_INLINE = 7, IDENT_OUTPUT_RUN = 17, IDENT_OUTPUT_CONTROL = 18 };
enum { FORMAT_S16 = 0, FORMAT_S32 = 1, FORMAT_FLOAT = 2 };

#define RUN_CODEC_INFO(f) (((f) >> 11) & 1)
#define RUN_MULTIPLE_FRAMES(f) (((f) >> 12) & 1)
#define CONTROL_RESET(f) (((f) >> 13) & 1)
#define CONTROL_INITIALIZE(f) (((f) >> 14) & 1)
#define CONTROL_RESAMPLE(f) (((f) >> 15) & 1)
#define SIDEBAND_GAPLESS(f) (((f) >> 45) & 1)
#define SIDEBAND_FORMAT(f) (((f) >> 46) & 1)
#define SIDEBAND_STREAM(f) (((f) >> 47) & 1)

typedef struct { uint32_t word, size; } Chunk;
typedef struct { uint32_t word, size; void *address; } ChunkBuffer;
typedef struct { int32_t result, internal_result; } SidebandResult;
typedef struct { int32_t input_consumed, output_written; uint64_t total_decoded_samples; } SidebandStream;
typedef struct { uint32_t channels, channel_mask, sample_rate, encoding, bitrate, reserved; } SidebandFormat;
typedef struct { uint32_t total_samples; uint16_t skip_samples, skipped_samples; } SidebandGapless;
typedef struct { uint32_t super_frame_size, frames_in_super_frame, next_frame_size, frame_samples; } At9Info;
typedef struct { int error_code; const void *job_address; uint32_t command_offset; const void *job_return_address; } BatchError;
typedef struct { int channels, channelConfigIndex, samplingRate, superframeSize, framesInSuperframe, frameBytes, frameSamples, isEntireSuperframe; } BbAtrac9CodecInfo;
typedef struct {
    int used, codec, format, channels_hint, gapless_loop;
    uint32_t codec_flags; void *handle; unsigned char config[4]; int initialized;
    BbAtrac9CodecInfo info; uint32_t superframe_remain, frames;
    SidebandGapless gapless_init, gapless; uint64_t total_samples;
} Instance;
typedef struct { int used, registered[24]; Instance instances[MAX_INSTANCES + 1]; } Context;
typedef struct { int used, context, canceled; } Batch;
typedef struct { uint64_t flags; int have_flags; ChunkBuffer inputs[16]; int input_count; ChunkBuffer outputs[16]; int output_count; ChunkBuffer input_control, output_control; } Job;
typedef struct { ChunkBuffer *buffers; int count, buffer_index; uint64_t offset; } Output;

void r_ajm_lock(void); void r_ajm_unlock(void); Context *r_ajm_context(uint32_t id);
uint32_t r_ajm_ident(uint32_t word); uint32_t r_ajm_payload(uint32_t word);
uint32_t r_ajm_header(uint32_t id, uint32_t value); int r_ajm_pcm_size(int format);

extern Context *g_ajm_contexts[MAX_CONTEXTS + 1];
extern Batch g_ajm_batches[MAX_BATCHES];
extern size_t g_ajm_jobs_run, g_ajm_frames_decoded, g_ajm_batches_run;

#endif /* R_AJM_TYPES_H */
