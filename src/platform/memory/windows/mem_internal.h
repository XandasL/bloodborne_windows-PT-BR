/* SPDX-License-Identifier: MIT
 * Windows Direct Memory Pool Internal Structures.
 * Single responsibility: Unified private layout for BbDirectPool. (~20 LOC)
 */
#ifndef MEM_INTERNAL_WINDOWS_H
#define MEM_INTERNAL_WINDOWS_H

#ifdef _WIN32
#include <windows.h>
#include <stddef.h>

struct BbDirectPool {
    HANDLE section_handle;
    void*  backing_base;
    size_t pool_size;
};
#endif

#endif /* MEM_INTERNAL_WINDOWS_H */
