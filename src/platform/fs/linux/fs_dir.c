/* SPDX-License-Identifier: MIT
 * Linux POSIX directory enumeration using opendir/readdir.
 * Single responsibility: Directory walking and entry metadata. (~70 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/fs.h"
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

struct BbDir {
    DIR* handle;
};

BbDir* bb_platform_dir_open(const char* host_path) {
    DIR* d = opendir(host_path);
    if (!d) return NULL;
    BbDir* dir = (BbDir*)malloc(sizeof(BbDir));
    if (!dir) { closedir(d); return NULL; }
    dir->handle = d;
    return dir;
}

bool bb_platform_dir_next(BbDir* dir, BbDirEntry* out_entry) {
    if (!dir || !out_entry) return false;
    struct dirent* entry;
    while ((entry = readdir(dir->handle)) != NULL) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) continue;
        snprintf(out_entry->name, sizeof(out_entry->name), "%s", entry->d_name);
        out_entry->is_directory = (entry->d_type == DT_DIR);
        out_entry->file_size = 0;
        return true;
    }
    return false;
}

void bb_platform_dir_close(BbDir* dir) {
    if (dir) {
        closedir(dir->handle);
        free(dir);
    }
}
#endif
