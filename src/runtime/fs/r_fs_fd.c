/* SPDX-License-Identifier: MIT
 * PS4 HLE file descriptor table management.
 * Single responsibility: Guest fd table allocation and tracking. (~65 LOC)
 */
#include "r_fs_types.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static File files[R_FS_MAX_FILES];
static size_t opens = 0;

File* r_fs_get_file(int fd) {
    if (fd < 3 || fd >= R_FS_MAX_FILES || !files[fd].used) return NULL;
    return &files[fd];
}

int r_fs_alloc_file(int64_t host_fd, Listing* dir, const char* guest_path) {
    int fd = -1;
    for (int i = 3; i < R_FS_MAX_FILES; ++i) {
        if (!files[i].used) {
            fd = i;
            break;
        }
    }
    if (fd < 0) return -1;

    files[fd].used = 1;
    files[fd].host = host_fd;
    files[fd].dir = dir;
    files[fd].position = 0;
    snprintf(files[fd].path, sizeof(files[fd].path), "%s", guest_path);
    ++opens;
    return fd;
}

void r_fs_free_file(int fd) {
    if (fd >= 3 && fd < R_FS_MAX_FILES) {
        if (files[fd].dir) {
            free(files[fd].dir->names);
            free(files[fd].dir->offsets);
            free(files[fd].dir->types);
            free(files[fd].dir);
        }
        memset(&files[fd], 0, sizeof(File));
    }
}

size_t r_fs_get_opens(void) {
    return opens;
}
