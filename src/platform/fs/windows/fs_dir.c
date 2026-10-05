/* SPDX-License-Identifier: MIT
 * Windows directory enumeration using FindFirstFileW / FindNextFileW.
 * Single responsibility: Directory walking and entry metadata. (~75 LOC)
 */
#ifdef _WIN32
#include "platform/fs.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

struct BbDir {
    HANDLE handle;
    WIN32_FIND_DATAW data;
    bool has_next;
};

BbDir* bb_platform_dir_open(const char* host_path) {
    char search_pattern[MAX_PATH];
    snprintf(search_pattern, sizeof(search_pattern), "%s/*", host_path);
    wchar_t wpattern[MAX_PATH];
    if (!MultiByteToWideChar(CP_UTF8, 0, search_pattern, -1, wpattern, MAX_PATH)) return NULL;

    BbDir* dir = (BbDir*)calloc(1, sizeof(BbDir));
    if (!dir) return NULL;
    dir->handle = FindFirstFileW(wpattern, &dir->data);
    if (dir->handle == INVALID_HANDLE_VALUE) {
        free(dir);
        return NULL;
    }
    dir->has_next = true;
    return dir;
}

bool bb_platform_dir_next(BbDir* dir, BbDirEntry* out_entry) {
    if (!dir || !dir->has_next || !out_entry) return false;
    while (dir->has_next) {
        bool skip = (wcscmp(dir->data.cFileName, L".") == 0 || wcscmp(dir->data.cFileName, L"..") == 0);
        if (!skip) {
            WideCharToMultiByte(CP_UTF8, 0, dir->data.cFileName, -1, out_entry->name, sizeof(out_entry->name), NULL, NULL);
            out_entry->is_directory = (dir->data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
            out_entry->file_size = ((uint64_t)dir->data.nFileSizeHigh << 32) | dir->data.nFileSizeLow;
            dir->has_next = FindNextFileW(dir->handle, &dir->data) != 0;
            return true;
        }
        dir->has_next = FindNextFileW(dir->handle, &dir->data) != 0;
    }
    return false;
}

void bb_platform_dir_close(BbDir* dir) {
    if (dir) {
        if (dir->handle != INVALID_HANDLE_VALUE) FindClose(dir->handle);
        free(dir);
    }
}
#endif
