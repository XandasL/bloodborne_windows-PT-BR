/* SPDX-License-Identifier: MIT
 * PS4 HLE mount table management.
 * Single responsibility: Guest-to-host mount points configuration. (~75 LOC)
 */
#include "r_fs_types.h"
#include <string.h>
#include <stdio.h>

static Mount mounts[R_FS_MAX_MOUNTS];
static size_t mount_count = 0;
static char user_root[512] = "user";

const char* runtime_file_user_dir(void) {
    return user_root;
}

int runtime_file_mount(const char* guest, const char* host) {
    for (size_t i = 0; i < mount_count; ++i) {
        if (!strcmp(mounts[i].guest, guest)) {
            snprintf(mounts[i].host, sizeof(mounts[i].host), "%s", host);
            return 0;
        }
    }
    if (mount_count == R_FS_MAX_MOUNTS || strlen(guest) >= 64 || strlen(host) >= 512) {
        return -1;
    }
    snprintf(mounts[mount_count].guest, 64, "%s", guest);
    snprintf(mounts[mount_count].host, 512, "%s", host);
    ++mount_count;
    return 0;
}

void runtime_file_unmount(const char* guest) {
    for (size_t i = 0; i < mount_count; ++i) {
        if (!strcmp(mounts[i].guest, guest)) {
            mounts[i] = mounts[--mount_count];
            break;
        }
    }
}

void runtime_file_configure(const char* app0, const char* user) {
    char path[600];
    snprintf(user_root, sizeof(user_root), "%s", user);
    runtime_file_mount("/app0", app0);
    runtime_file_mount("/hostapp", app0);
    const char* writable[] = {"temp0", "download0", "data"};
    bb_platform_mkdir(user);
    for (int i = 0; i < 3; ++i) {
        snprintf(path, sizeof(path), "%s/%s", user, writable[i]);
        bb_platform_mkdir(path);
        char guest[32];
        snprintf(guest, sizeof(guest), "/%s", writable[i]);
        runtime_file_mount(guest, path);
    }
}

size_t r_fs_get_mounts(Mount** out) {
    if (out) *out = mounts;
    return mount_count;
}
