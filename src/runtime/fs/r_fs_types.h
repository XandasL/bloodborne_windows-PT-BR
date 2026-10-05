/* SPDX-License-Identifier: MIT
 * PS4 HLE filesystem types and structures.
 * Single responsibility: FreeBSD/Orbis stat layout and descriptor structures. (~50 LOC)
 */
#ifndef R_FS_TYPES_H
#define R_FS_TYPES_H

#include "bb_common.h"
#include "platform/fs.h"
#include "platform/sync.h"

#define R_FS_MAX_FILES 1024
#define R_FS_MAX_MOUNTS 16

typedef struct { int64_t sec, nsec; } GuestTimespec;

typedef struct {
    uint32_t dev, ino;
    uint16_t mode, nlink;
    uint32_t uid, gid, rdev;
    GuestTimespec atime, mtime, ctime;
    int64_t size, blocks;
    uint32_t blksize, flags, gen;
    int32_t lspare;
    GuestTimespec birthtime;
} GuestStat;
_Static_assert(sizeof(GuestStat) == 120, "FreeBSD stat layout");

typedef struct { char* names; size_t count, *offsets; unsigned char* types; } Listing;
typedef struct { int used; int64_t host; Listing* dir; size_t position; char path[512]; } File;
typedef struct { char guest[64]; char host[512]; } Mount;

int r_fs_translate(const char* guest, char* out, size_t size);
int r_fs_is_game_path(const char* p);
File* r_fs_get_file(int fd);
int r_fs_alloc_file(int64_t host_fd, Listing* dir, const char* guest_path);
void r_fs_free_file(int fd);

#endif /* R_FS_TYPES_H */
