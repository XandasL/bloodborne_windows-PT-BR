/* SPDX-License-Identifier: MIT
 * Linux Direct Memory Pool Internal Structures.
 * Single responsibility: Unified private layout for BbDirectPool. (~20 LOC)
 */
#ifndef MEM_INTERNAL_LINUX_H
#define MEM_INTERNAL_LINUX_H

#ifndef _WIN32
#include <stddef.h>

struct BbDirectPool {
    int    fd;
    void*  backing_base;
    size_t pool_size;
};
#endif

#endif /* MEM_INTERNAL_LINUX_H */
