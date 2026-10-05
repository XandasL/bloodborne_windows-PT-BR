/* SPDX-License-Identifier: MIT
 * Windows filesystem stat and directory manipulation.
 * Single responsibility: File attributes, creation, deletion. (~65 LOC)
 */
#ifdef _WIN32
#include "platform/fs.h"
#include <windows.h>

int32_t bb_platform_file_stat(const char* host_path, uint64_t* size, uint64_t* mtime_sec, bool* is_dir) {
    wchar_t wpath[MAX_PATH];
    if (!MultiByteToWideChar(CP_UTF8, 0, host_path, -1, wpath, MAX_PATH)) return -1;
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExW(wpath, GetFileExInfoStandard, &data)) return -1;

    if (size) *size = ((uint64_t)data.nFileSizeHigh << 32) | data.nFileSizeLow;
    if (is_dir) *is_dir = (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    if (mtime_sec) {
        ULARGE_INTEGER ft;
        ft.LowPart = data.ftLastWriteTime.dwLowDateTime;
        ft.HighPart = data.ftLastWriteTime.dwHighDateTime;
        *mtime_sec = (ft.QuadPart / 10000000ULL) - 11644473600ULL;
    }
    return 0;
}

int32_t bb_platform_mkdir(const char* host_path) {
    wchar_t wpath[MAX_PATH];
    if (!MultiByteToWideChar(CP_UTF8, 0, host_path, -1, wpath, MAX_PATH)) return -1;
    if (CreateDirectoryW(wpath, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        return 0;
    }
    return -1;
}

int32_t bb_platform_remove(const char* host_path) {
    wchar_t wpath[MAX_PATH];
    if (!MultiByteToWideChar(CP_UTF8, 0, host_path, -1, wpath, MAX_PATH)) return -1;
    DWORD attr = GetFileAttributesW(wpath);
    if (attr == INVALID_FILE_ATTRIBUTES) return -1;
    if (attr & FILE_ATTRIBUTE_DIRECTORY) {
        return RemoveDirectoryW(wpath) ? 0 : -1;
    }
    return DeleteFileW(wpath) ? 0 : -1;
}
#endif
