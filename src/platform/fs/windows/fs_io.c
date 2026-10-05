/* SPDX-License-Identifier: MIT
 * Windows file I/O operations using Win32 CreateFileW/ReadFile/WriteFile.
 * Single responsibility: Basic file descriptor I/O and positioning. (~75 LOC)
 */
#ifdef _WIN32
#include "platform/fs.h"
#include <windows.h>

int64_t bb_platform_file_open(const char* host_path, int flags, int mode) {
    BB_UNUSED(mode);
    wchar_t wpath[MAX_PATH];
    if (!MultiByteToWideChar(CP_UTF8, 0, host_path, -1, wpath, MAX_PATH)) return -1;
    DWORD access = GENERIC_READ;
    DWORD disposition = OPEN_EXISTING;
    if (flags & 1) access = GENERIC_WRITE;
    else if (flags & 2) access = GENERIC_READ | GENERIC_WRITE;
    if (flags & 0x200) disposition = (flags & 0x800) ? CREATE_NEW : OPEN_ALWAYS;
    if (flags & 0x400) disposition = TRUNCATE_EXISTING;

    HANDLE h = CreateFileW(wpath, access, FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL, disposition, FILE_ATTRIBUTE_NORMAL, NULL);
    return (h == INVALID_HANDLE_VALUE) ? -1 : (int64_t)(intptr_t)h;
}

int64_t bb_platform_file_read(int64_t fd, void* buffer, uint64_t size) {
    DWORD read_bytes = 0;
    if (!ReadFile((HANDLE)(intptr_t)fd, buffer, (DWORD)size, &read_bytes, NULL)) return -1;
    return (int64_t)read_bytes;
}

int64_t bb_platform_file_pread(int64_t fd, void* buffer, uint64_t size, int64_t offset) {
    OVERLAPPED ov = {0};
    ov.Offset = (DWORD)offset;
    ov.OffsetHigh = (DWORD)(offset >> 32);
    DWORD read_bytes = 0;
    if (!ReadFile((HANDLE)(intptr_t)fd, buffer, (DWORD)size, &read_bytes, &ov)) {
        if (GetLastError() != ERROR_HANDLE_EOF) return -1;
    }
    return (int64_t)read_bytes;
}

int64_t bb_platform_file_write(int64_t fd, const void* buffer, uint64_t size) {
    DWORD written = 0;
    if (!WriteFile((HANDLE)(intptr_t)fd, buffer, (DWORD)size, &written, NULL)) return -1;
    return (int64_t)written;
}

int64_t bb_platform_file_seek(int64_t fd, int64_t offset, int whence) {
    LARGE_INTEGER dist, result;
    dist.QuadPart = offset;
    DWORD method = (whence == 1) ? FILE_CURRENT : (whence == 2) ? FILE_END : FILE_BEGIN;
    if (!SetFilePointerEx((HANDLE)(intptr_t)fd, dist, &result, method)) return -1;
    return result.QuadPart;
}

int32_t bb_platform_file_truncate(int64_t fd, uint64_t size) {
    LARGE_INTEGER li;
    li.QuadPart = (LONGLONG)size;
    if (!SetFilePointerEx((HANDLE)(intptr_t)fd, li, NULL, FILE_BEGIN)) return -1;
    return SetEndOfFile((HANDLE)(intptr_t)fd) ? 0 : -1;
}

int64_t bb_platform_file_close(int64_t fd) {
    return CloseHandle((HANDLE)(intptr_t)fd) ? 0 : -1;
}
#endif
