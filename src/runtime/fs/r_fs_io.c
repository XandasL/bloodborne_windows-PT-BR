/* SPDX-License-Identifier: MIT
 * PS4 HLE file input/output operations.
 * Single responsibility: Guest open/read/write/seek implementations. (~75 LOC)
 */
#include "r_fs_types.h"

int64_t runtime_file_open(const char* guest, int flags, int mode) {
    if (r_fs_is_game_path(guest) && (flags & (3 | 0x8 | 0x200 | 0x400 | 0x800))) {
        return -30; /* EROFS */
    }
    char path[1024];
    int err = r_fs_translate(guest, path, sizeof(path));
    if (err) return -err;

    int64_t host = bb_platform_file_open(path, flags, mode);
    if (host < 0) return -2; /* ENOENT */

    int fd = r_fs_alloc_file(host, NULL, guest);
    if (fd < 0) {
        bb_platform_file_close(host);
        return -24; /* EMFILE */
    }
    return fd;
}

int64_t runtime_file_close(int fd) {
    if (fd >= 0 && fd < 3) return 0;
    File* f = r_fs_get_file(fd);
    if (!f) return -9; /* EBADF */
    bb_platform_file_close(f->host);
    r_fs_free_file(fd);
    return 0;
}

int64_t runtime_file_read(int fd, void* buffer, uint64_t size) {
    File* f = r_fs_get_file(fd);
    if (!f) return -9;
    return bb_platform_file_read(f->host, buffer, size);
}

int64_t runtime_file_pread(int fd, void* buffer, uint64_t size, int64_t offset) {
    File* f = r_fs_get_file(fd);
    if (!f) return -9;
    return bb_platform_file_pread(f->host, buffer, size, offset);
}

int64_t runtime_file_write(int fd, const void* buffer, uint64_t size) {
    File* f = r_fs_get_file(fd);
    if (!f) return -9;
    return bb_platform_file_write(f->host, buffer, size);
}

int64_t runtime_file_lseek(int fd, int64_t offset, int whence) {
    File* f = r_fs_get_file(fd);
    if (!f) return -9;
    return bb_platform_file_seek(f->host, offset, whence);
}
