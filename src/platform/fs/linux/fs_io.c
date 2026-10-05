/* SPDX-License-Identifier: MIT
 * Linux POSIX file I/O operations.
 * Single responsibility: Basic POSIX open/read/write/seek. (~65 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/fs.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int64_t bb_platform_file_open(const char* host_path, int flags, int mode) {
    int host_flags = O_RDONLY;
    if (flags & 1) host_flags = O_WRONLY;
    else if (flags & 2) host_flags = O_RDWR;
    if (flags & 0x200) host_flags |= O_CREAT;
    if (flags & 0x400) host_flags |= O_TRUNC;
    if (flags & 0x800) host_flags |= O_EXCL;
    return open(host_path, host_flags | O_CLOEXEC, mode ? mode : 0644);
}

int64_t bb_platform_file_read(int64_t fd, void* buffer, uint64_t size) {
    return read((int)fd, buffer, size);
}

int64_t bb_platform_file_pread(int64_t fd, void* buffer, uint64_t size, int64_t offset) {
    return pread((int)fd, buffer, size, offset);
}

int64_t bb_platform_file_write(int64_t fd, const void* buffer, uint64_t size) {
    return write((int)fd, buffer, size);
}

int64_t bb_platform_file_pwrite(int64_t fd, const void* buffer, uint64_t size, int64_t offset) {
    return pwrite((int)fd, buffer, size, offset);
}

int64_t bb_platform_file_seek(int64_t fd, int64_t offset, int whence) {
    int w = (whence == 1) ? SEEK_CUR : (whence == 2) ? SEEK_END : SEEK_SET;
    return lseek((int)fd, offset, w);
}

int32_t bb_platform_file_truncate(int64_t fd, uint64_t size) {
    return ftruncate((int)fd, (off_t)size);
}

int64_t bb_platform_file_close(int64_t fd) {
    return close((int)fd);
}
#endif
