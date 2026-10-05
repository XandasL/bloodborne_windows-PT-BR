/* SPDX-License-Identifier: MIT
 * PS4 HLE filesystem NID resolver and diagnostic reporting.
 * Single responsibility: Export table mapping and report generation. (~75 LOC)
 */
#include "runtime.h"
#include "r_fs_types.h"
#include <stdio.h>
#include <string.h>

size_t r_fs_get_opens(void);

static BB_SYSV_ABI int64_t sce_open(const char* p, int f, int m) {
    int64_t r = runtime_file_open(p, f, m);
    return r < 0 ? BB_ORBIS_ERROR(runtime_guest_errno((int)-r)) : r;
}

static BB_SYSV_ABI int64_t sce_close(int fd) {
    int64_t r = runtime_file_close(fd);
    return r < 0 ? BB_ORBIS_ERROR(runtime_guest_errno((int)-r)) : r;
}

static BB_SYSV_ABI int64_t sce_read(int fd, void* b, uint64_t n) {
    int64_t r = runtime_file_read(fd, b, n);
    return r < 0 ? BB_ORBIS_ERROR(runtime_guest_errno((int)-r)) : r;
}

static BB_SYSV_ABI int64_t sce_write(int fd, const void* b, uint64_t n) {
    int64_t r = runtime_file_write(fd, b, n);
    return r < 0 ? BB_ORBIS_ERROR(runtime_guest_errno((int)-r)) : r;
}

static BB_SYSV_ABI int64_t sce_lseek(int fd, int64_t o, int w) {
    int64_t r = runtime_file_lseek(fd, o, w);
    return r < 0 ? BB_ORBIS_ERROR(runtime_guest_errno((int)-r)) : r;
}

static BB_SYSV_ABI int64_t sce_fstat(int fd, void* s) {
    int64_t r = runtime_file_fstat(fd, s);
    return r < 0 ? BB_ORBIS_ERROR(runtime_guest_errno((int)-r)) : r;
}

static BB_SYSV_ABI int64_t sce_stat(const char* p, void* s) {
    int64_t r = runtime_file_stat(p, s);
    return r < 0 ? BB_ORBIS_ERROR(runtime_guest_errno((int)-r)) : r;
}

uintptr_t runtime_file_resolve(const char* name) {
    if (!strcmp(name, "sceKernelOpen") || !strcmp(name, "open") || !strcmp(name, "_open")) return (uintptr_t)sce_open;
    if (!strcmp(name, "sceKernelClose") || !strcmp(name, "close") || !strcmp(name, "_close")) return (uintptr_t)sce_close;
    if (!strcmp(name, "sceKernelRead") || !strcmp(name, "read") || !strcmp(name, "_read")) return (uintptr_t)sce_read;
    if (!strcmp(name, "sceKernelWrite") || !strcmp(name, "write") || !strcmp(name, "_write")) return (uintptr_t)sce_write;
    if (!strcmp(name, "sceKernelLseek") || !strcmp(name, "lseek") || !strcmp(name, "_lseek")) return (uintptr_t)sce_lseek;
    if (!strcmp(name, "sceKernelFstat") || !strcmp(name, "fstat")) return (uintptr_t)sce_fstat;
    if (!strcmp(name, "sceKernelStat") || !strcmp(name, "stat")) return (uintptr_t)sce_stat;
    return 0;
}

void runtime_file_report(void) {
    printf("Runtime: filesystem active (files opened=%zu)\n", r_fs_get_opens());
}
