/* SPDX-License-Identifier: MIT
 * PS4 SaveData Memory Allocation and Fixed-Size Blob I/O.
 * Single responsibility: Fixed-size save data memory persistence and I/O. (~65 LOC)
 */
#include "r_savedata_types.h"

static char memory_path[720];
static uint64_t memory_size = 0;

ABI int32_t memory_setup(int32_t user, uint64_t size, const Param *param) {
    if (!g_savedata_initialized) return ERR_NOT_INITIALIZED;
    if (!size) return ERR_PARAMETER;
    char base[600], dir[640];
    r_savedata_root(user, NULL, base, sizeof(base));
    snprintf(dir, sizeof(dir), "%s.memory", base);
    if (r_savedata_make_dirs(dir)) return ERR_INTERNAL;
    r_savedata_lock();
    snprintf(memory_path, sizeof(memory_path), "%s/memory.dat", dir);
    int64_t fd = bb_platform_file_open(memory_path, 2 | 0x200, 0644);
    int32_t r = ERR_INTERNAL;
    if (fd >= 0) {
        int64_t cur = bb_platform_file_seek(fd, 0, 2);
        if ((uint64_t)cur < size && bb_platform_file_truncate(fd, size) != 0) goto done;
        memory_size = size; r = 0;
    done:
        bb_platform_file_close(fd);
    }
    if (!r && param) r_savedata_write_param(dir, param);
    r_savedata_unlock();
    if (!r) printf("Runtime: save data memory ready (%" PRIu64 " bytes)\n", size);
    return r;
}

static int32_t memory_io(void *buffer, uint64_t size, int64_t offset, int write) {
    if (!g_savedata_initialized) return ERR_NOT_INITIALIZED;
    if (!buffer || offset < 0) return ERR_PARAMETER;
    r_savedata_lock();
    if (!memory_size) { r_savedata_unlock(); return ERR_MEMORY_NOT_READY; }
    if ((uint64_t)offset + size > memory_size) { r_savedata_unlock(); return ERR_PARAMETER; }
    int64_t fd = bb_platform_file_open(memory_path, write ? 2 : 0, 0644);
    int32_t r = ERR_INTERNAL;
    if (fd >= 0) {
        if (bb_platform_file_seek(fd, offset, 0) == offset) {
            int64_t rw = write ? bb_platform_file_write(fd, buffer, size)
                               : bb_platform_file_read(fd, buffer, size);
            if (rw == (int64_t)size) r = 0;
        }
        bb_platform_file_close(fd);
    }
    if (!r && write) ++g_savedata_memory_writes;
    r_savedata_unlock();
    return r;
}

ABI int32_t memory_get(int32_t user, void *buffer, uint64_t size, int64_t offset) {
    (void)user; return memory_io(buffer, size, offset, 0);
}
ABI int32_t memory_set(int32_t user, void *buffer, uint64_t size, int64_t offset) {
    (void)user; return memory_io(buffer, size, offset, 1);
}
