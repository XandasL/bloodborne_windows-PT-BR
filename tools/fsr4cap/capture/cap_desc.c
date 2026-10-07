// SPDX-License-Identifier: GPL-2.0-or-later
#include "cap_types.h"

static Heap heaps[64];
static int heap_count;
static Pso psos[1024];
static int pso_count;
static Root roots[256];
static int root_count;
static ListState lists[16];

static View* ViewAtCpu(SIZE_T cpu) {
    for (int i = 0; i < heap_count; ++i) {
        Heap* h = &heaps[i];
        if (cpu >= h->cpu && cpu < h->cpu + (SIZE_T)h->count * h->inc) return &h->views[(cpu - h->cpu) / h->inc];
    }
    return NULL;
}

static View* ViewAtGpu(UINT64 gpu, Heap** heap) {
    for (int i = 0; i < heap_count; ++i) {
        Heap* h = &heaps[i];
        if (h->gpu && gpu >= h->gpu && gpu < h->gpu + (UINT64)h->count * h->inc) {
            *heap = h;
            return &h->views[(gpu - h->gpu) / h->inc];
        }
    }
    return NULL;
}

static uint64_t RootHash(void* obj) {
    for (int i = 0; i < root_count; ++i) if (roots[i].obj == obj) return roots[i].hash;
    return 0;
}

static uint64_t PsoHash(void* pso) {
    for (int i = 0; i < pso_count; ++i) if (psos[i].obj == pso) return psos[i].hash;
    return 0;
}

static ListState* State(void* list) {
    for (int i = 0; i < 16; ++i) if (lists[i].list == list) return &lists[i];
    for (int i = 0; i < 16; ++i) if (!lists[i].list) { lists[i].list = list; return &lists[i]; }
    return &lists[0];
}
