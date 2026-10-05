/* SPDX-License-Identifier: MIT
 * PS4 Virtual Memory Range Table (VMA) Interval Operations.
 * Single responsibility: VMA interval search, insertion, and splitting. (~65 LOC)
 */
#include "r_mem_types.h"

Vma *g_mem_vmas = NULL;
size_t g_mem_vma_count = 0, g_mem_vma_capacity = 0;

size_t r_mem_vma_index(uintptr_t a) {
    size_t lo = 0, hi = g_mem_vma_count;
    while (lo < hi) {
        size_t mid = (lo + hi) / 2;
        if (g_mem_vmas[mid].end <= a) lo = mid + 1;
        else hi = mid;
    }
    return lo;
}

int r_mem_vma_insert(size_t at, Vma v) {
    if (g_mem_vma_count == g_mem_vma_capacity) {
        size_t cap = g_mem_vma_capacity ? g_mem_vma_capacity * 2 : 256;
        Vma *next = (Vma*)realloc(g_mem_vmas, cap * sizeof(*g_mem_vmas));
        if (!next) return -1;
        g_mem_vmas = next;
        g_mem_vma_capacity = cap;
    }
    memmove(g_mem_vmas + at + 1, g_mem_vmas + at, (g_mem_vma_count - at) * sizeof(*g_mem_vmas));
    g_mem_vmas[at] = v;
    ++g_mem_vma_count;
    return 0;
}

void r_mem_vma_erase(size_t at, size_t n) {
    memmove(g_mem_vmas + at, g_mem_vmas + at + n, (g_mem_vma_count - at - n) * sizeof(*g_mem_vmas));
    g_mem_vma_count -= n;
}

static int split_at(uintptr_t a) {
    size_t i = r_mem_vma_index(a);
    if (i == g_mem_vma_count || g_mem_vmas[i].start >= a) return 0;
    Vma right = g_mem_vmas[i];
    right.start = a;
    if (right.kind == KIND_DIRECT) right.phys += a - g_mem_vmas[i].start;
    g_mem_vmas[i].end = a;
    return r_mem_vma_insert(i + 1, right);
}

size_t r_mem_carve(uintptr_t start, uintptr_t end, int *error) {
    *error = split_at(start) || split_at(end);
    return r_mem_vma_index(start);
}
