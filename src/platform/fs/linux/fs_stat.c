/* SPDX-License-Identifier: MIT
 * Linux POSIX file stat and directory management.
 * Single responsibility: POSIX stat, mkdir, unlink. (~60 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/fs.h"
#include <sys/stat.h>
#include <unistd.h>

int32_t bb_platform_file_stat(const char* host_path, uint64_t* size, uint64_t* mtime_sec, bool* is_dir) {
    struct stat st;
    if (stat(host_path, &st) != 0) return -1;
    if (size) *size = (uint64_t)st.st_size;
    if (is_dir) *is_dir = S_ISDIR(st.st_mode);
    if (mtime_sec) *mtime_sec = (uint64_t)st.st_mtime;
    return 0;
}

int32_t bb_platform_mkdir(const char* host_path) {
    return mkdir(host_path, 0755);
}

int32_t bb_platform_remove(const char* host_path) {
    struct stat st;
    if (stat(host_path, &st) != 0) return -1;
    if (S_ISDIR(st.st_mode)) return rmdir(host_path);
    return unlink(host_path);
}
#endif
