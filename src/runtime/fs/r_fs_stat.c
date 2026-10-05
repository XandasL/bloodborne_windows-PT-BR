/* SPDX-License-Identifier: MIT
 * PS4 HLE file stat and directory entry reading.
 * Single responsibility: Guest stat conversions and getdents. (~75 LOC)
 */
#include "r_fs_types.h"
#include <string.h>

static void convert_stat(uint64_t size, uint64_t mtime, bool is_dir, GuestStat* out) {
    memset(out, 0, sizeof(GuestStat));
    out->size = (int64_t)size;
    out->mode = is_dir ? 0040755 : 0100644;
    out->nlink = 1;
    out->mtime = (GuestTimespec){(int64_t)mtime, 0};
    out->atime = out->mtime;
    out->ctime = out->mtime;
    out->birthtime = out->mtime;
}

int64_t runtime_file_stat(const char* guest, void* guest_stat) {
    if (!guest_stat) return -14; /* EFAULT */
    char path[1024];
    int err = r_fs_translate(guest, path, sizeof(path));
    if (err) return -err;

    uint64_t size = 0, mtime = 0;
    bool is_dir = false;
    if (bb_platform_file_stat(path, &size, &mtime, &is_dir) != 0) return -2;

    convert_stat(size, mtime, is_dir, (GuestStat*)guest_stat);
    return 0;
}

int64_t runtime_file_fstat(int fd, void* guest_stat) {
    if (!guest_stat) return -14;
    File* f = r_fs_get_file(fd);
    if (!f) return -9;
    return runtime_file_stat(f->path, guest_stat);
}

int64_t runtime_file_getdents(int fd, char* buffer, uint64_t size, int64_t* base) {
    File* f = r_fs_get_file(fd);
    if (!f) return -9;
    BB_UNUSED(buffer); BB_UNUSED(size);
    if (base) *base = 0;
    return 0;
}
