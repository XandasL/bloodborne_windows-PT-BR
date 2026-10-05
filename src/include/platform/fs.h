/* SPDX-License-Identifier: MIT
 * Platform filesystem abstraction interface.
 * Single responsibility: Cross-platform file I/O and directory walk. (~55 LOC)
 */
#ifndef BB_PLATFORM_FS_H
#define BB_PLATFORM_FS_H

#include "bb_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BbDir BbDir;
typedef struct BbDirEntry {
    char name[256];
    bool is_directory;
    uint64_t file_size;
} BbDirEntry;

int64_t  bb_platform_file_open(const char* host_path, int flags, int mode);
int64_t  bb_platform_file_read(int64_t fd, void* buffer, uint64_t size);
int64_t  bb_platform_file_pread(int64_t fd, void* buffer, uint64_t size, int64_t offset);
int64_t  bb_platform_file_write(int64_t fd, const void* buffer, uint64_t size);
int64_t  bb_platform_file_pwrite(int64_t fd, const void* buffer, uint64_t size, int64_t offset);
int64_t  bb_platform_file_seek(int64_t fd, int64_t offset, int whence);
int32_t  bb_platform_file_truncate(int64_t fd, uint64_t size);
int64_t  bb_platform_file_close(int64_t fd);

int32_t  bb_platform_file_stat(const char* host_path, uint64_t* size, uint64_t* mtime_sec, bool* is_dir);
int32_t  bb_platform_mkdir(const char* host_path);
int32_t  bb_platform_remove(const char* host_path);

BbDir*   bb_platform_dir_open(const char* host_path);
bool     bb_platform_dir_next(BbDir* dir, BbDirEntry* out_entry);
void     bb_platform_dir_close(BbDir* dir);

#ifdef __cplusplus
}
#endif

#endif /* BB_PLATFORM_FS_H */
