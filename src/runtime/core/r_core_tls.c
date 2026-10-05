/* SPDX-License-Identifier: MIT
 * PS4 Core Dynamic TLS and Process Parameter Management.
 * Single responsibility: Multi-module TLS allocations and procparam anchor. (~55 LOC)
 */
#include "r_core_types.h"

static struct { const void *data; uint64_t filesz, memsz; } tls_modules[TLS_MODULES];
static _Thread_local unsigned char *tls_blocks[TLS_MODULES];
static void *process_param;
static void **application_heap_api;

void runtime_set_procparam(void *param) { process_param = param; }
ABI void *guest_procparam(void) { return process_param; }

ABI void guest_set_heap_api(void **api) {
    application_heap_api = api;
    puts("Runtime: application heap API registered");
}
void **runtime_application_heap_api(void) { return application_heap_api; }

void runtime_set_module_tls(uint64_t module, const void *data, uint64_t filesz, uint64_t memsz) {
    if (module < 2 || module >= TLS_MODULES) { fputs("STOP: unsupported TLS module id\n", stderr); exit(21); }
    tls_modules[module].data = data;
    tls_modules[module].filesz = filesz;
    tls_modules[module].memsz = memsz;
}

void runtime_set_libc_tls(const void *data, uint64_t filesz, uint64_t memsz) {
    runtime_set_module_tls(2, data, filesz, memsz);
}

ABI void *guest_tls_get_addr(const uint64_t *index) {
    if (!index || index[0] < 2 || index[0] >= TLS_MODULES || !tls_modules[index[0]].data || index[1] >= tls_modules[index[0]].memsz) {
        fputs("STOP: unsupported TLS module/offset\n", stderr); exit(21);
    }
    unsigned char **block = &tls_blocks[index[0]];
    if (!*block) {
        *block = (unsigned char *)calloc(1, tls_modules[index[0]].memsz);
        if (!*block) { fputs("Cannot allocate module TLS\n", stderr); exit(1); }
        memcpy(*block, tls_modules[index[0]].data, tls_modules[index[0]].filesz);
    }
    return *block + index[1];
}
