/* SPDX-License-Identifier: MIT
 * PS4 Guest Thread TLS Block and TCB Configuration.
 * Single responsibility: FreeBSD-style TLS allocation and GS anchor. (~55 LOC)
 */
#include "r_thread_types.h"

static const unsigned char *tls_template;
static uint64_t tls_filesz, tls_memsz, tls_align = 16;
static _Thread_local GuestThread *s_current_thread;

void runtime_set_main_tls(const void *data, uint64_t filesz, uint64_t memsz, uint64_t align) {
    tls_template = data; tls_filesz = filesz; tls_memsz = memsz; tls_align = align ? align : 16;
}

static uint64_t tls_offset(void) {
    return (tls_memsz + tls_align - 1) & ~(tls_align - 1);
}

void r_thread_attach(GuestThread *t) {
    uint64_t offset = tls_offset();
    size_t total = (size_t)offset + 256;
    unsigned char *block = (unsigned char*)calloc(1, (total + 63) & ~(size_t)63);
    if (!block) { fputs("Cannot allocate guest TLS\n", stderr); exit(1); }
    if (tls_filesz) memcpy(block, tls_template, tls_filesz);
    uint64_t *tcb = (uint64_t*)(block + offset);
    static uint64_t dtv[3];
    tcb[0] = (uint64_t)(uintptr_t)tcb;
    tcb[1] = (uint64_t)(uintptr_t)dtv;
    tcb[2] = (uint64_t)(uintptr_t)t;
    t->tls_block = block; t->tcb = tcb;
    bb_platform_tls_set_guest(tcb);
    s_current_thread = t;
}

GuestThread *runtime_thread_current(void) {
    if (s_current_thread) return s_current_thread;
    GuestThread *t = r_thread_new();
    if (!t) { fputs("Cannot allocate guest thread\n", stderr); exit(1); }
    t->host_owned = 1;
    snprintf(t->name, sizeof(t->name), "host");
    r_thread_attach(t);
    r_thread_publish(t);
    return t;
}
