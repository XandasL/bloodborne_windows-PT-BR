/* SPDX-License-Identifier: MIT
 * PS4 Ajm Job Output Streams and Job Command Parser.
 * Single responsibility: Job command chunk parsing and output buffer handling. (~55 LOC)
 */
#include "r_ajm_types.h"

uint64_t r_ajm_output_room(const Output *o) {
    uint64_t room = 0;
    for (int i = o->buffer_index; i < o->count; ++i)
        room += o->buffers[i].size - (i == o->buffer_index ? o->offset : 0);
    return room;
}

void r_ajm_output_write(Output *o, const unsigned char *data, uint64_t size) {
    while (size && o->buffer_index < o->count) {
        ChunkBuffer *c = &o->buffers[o->buffer_index];
        uint64_t n = c->size - o->offset;
        if (n > size) n = size;
        memcpy((unsigned char*)c->address + o->offset, data, n);
        data += n; size -= n; o->offset += n;
        if (o->offset == c->size) { ++o->buffer_index; o->offset = 0; }
    }
}

int r_ajm_parse_job(const unsigned char *p, uint32_t size, Job *job) {
    memset(job, 0, sizeof(*job));
    for (const unsigned char *end = p + size; p < end;) {
        uint32_t word; memcpy(&word, p, 4);
        switch (r_ajm_ident(word)) {
        case IDENT_CONTROL_FLAGS: case IDENT_RUN_FLAGS: {
            Chunk c; memcpy(&c, p, 8);
            job->flags = ((uint64_t)r_ajm_payload(c.word) << 32) | c.size; job->have_flags = 1; p += 8; break;
        }
        case IDENT_INPUT_RUN: if (job->input_count == 16) return -1; memcpy(&job->inputs[job->input_count++], p, 16); p += 16; break;
        case IDENT_OUTPUT_RUN: if (job->output_count == 16) return -1; memcpy(&job->outputs[job->output_count++], p, 16); p += 16; break;
        case IDENT_INPUT_CONTROL: memcpy(&job->input_control, p, 16); p += 16; break;
        case IDENT_OUTPUT_CONTROL: memcpy(&job->output_control, p, 16); p += 16; break;
        case IDENT_RETURN_ADDRESS: p += 16; break;
        default: return -1;
        }
    }
    return job->have_flags ? 0 : -1;
}
