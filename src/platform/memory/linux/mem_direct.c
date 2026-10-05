/* SPDX-License-Identifier: MIT
 * Linux direct memory pool management using memfd_create.
 * Single responsibility: Anonymous shared memory file descriptor pool. (~65 LOC)
 */
#ifndef _WIN32
#define _GNU_SOURCE
#include "platform/memory.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

struct BbDirectPool {
    int fd;
    void* backing_base;
    size_t pool_size;
};

BbDirectPool* bb_platform_direct_create(size_t size) {
    BbDirectPool* pool = (BbDirectPool*)calloc(1, sizeof(BbDirectPool));
    if (!pool) return NULL;

    pool->fd = memfd_create("bb_direct_pool", MFD_CLOEXEC);
    if (pool->fd < 0 || ftruncate(pool->fd, (off_t)size) != 0) {
        if (pool->fd >= 0) close(pool->fd);
        free(pool);
        return NULL;
    }
    pool->backing_base = mmap(NULL, size, PROT_READ | PROT_WRITE,
                              MAP_SHARED, pool->fd, 0);
    if (pool->backing_base == MAP_FAILED) {
        close(pool->fd);
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
    if (!pool || pool->fd < 0) return;
#ifdef FALLOC_FL_PUNCH_HOLE
    fallocate(pool->fd, FALLOC_FL_PUNCH_HOLE | FALLOC_FL_KEEP_SIZE, (off_t)offset, (off_t)size);
#endif
}

void bb_platform_direct_destroy(BbDirectPool* pool) {
    if (pool) {
        if (pool->backing_base && pool->backing_base != MAP_FAILED) {
            munmap(pool->backing_base, pool->pool_size);
        }
        if (pool->fd >= 0) close(pool->fd);
        free(pool);
    }
}
#endif
