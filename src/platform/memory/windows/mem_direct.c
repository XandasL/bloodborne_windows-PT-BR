/* SPDX-License-Identifier: MIT
 * Windows direct memory pool management using Win32 Section Objects.
 * Single responsibility: Anonymous page-file section creation/destruction. (~65 LOC)
 */
#ifdef _WIN32
#include "platform/memory.h"
#include <windows.h>
#include <stdlib.h>
#include <string.h>

struct BbDirectPool {
    HANDLE section_handle;
    void* backing_base;
    size_t pool_size;
};

BbDirectPool* bb_platform_direct_create(size_t size) {
    BbDirectPool* pool = (BbDirectPool*)calloc(1, sizeof(BbDirectPool));
    if (!pool) return NULL;

    ULARGE_INTEGER li;
    li.QuadPart = size;

    pool->section_handle = CreateFileMappingW(
        INVALID_HANDLE_VALUE, NULL, PAGE_EXECUTE_READWRITE | SEC_COMMIT,
        li.HighPart, li.LowPart, NULL
    );
    if (!pool->section_handle) { free(pool); return NULL; }

    pool->backing_base = MapViewOfFile(pool->section_handle, FILE_MAP_ALL_ACCESS, 0, 0, size);
    if (!pool->backing_base) {
        CloseHandle(pool->section_handle);
        free(pool);
        return NULL;
    }
    pool->pool_size = size;
    return pool;
}

void* bb_platform_direct_backing_base(BbDirectPool* pool) {
    return pool ? pool->backing_base : NULL;
}

void bb_platform_direct_punch_hole(BbDirectPool* pool, uint64_t offset, size_t size) {
    if (!pool || !pool->backing_base || offset + size > pool->pool_size) return;
    memset((char*)pool->backing_base + offset, 0, size);
}

void bb_platform_direct_destroy(BbDirectPool* pool) {
    if (pool) {
        if (pool->backing_base) UnmapViewOfFile(pool->backing_base);
        if (pool->section_handle) CloseHandle(pool->section_handle);
        free(pool);
    }
}
#endif
