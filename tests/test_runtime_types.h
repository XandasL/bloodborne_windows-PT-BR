#pragma once
#define _GNU_SOURCE
#include "runtime.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdatomic.h>
#include <time.h>

void runtime_restart(void) { abort(); }

typedef int (ABI *Register)(GuestCallback);
typedef int (ABI *RegisterCxa)(void (ABI *)(void *), void *, void *);
typedef int (ABI *Acquire)(uint64_t *);
typedef void (ABI *Guard)(uint64_t *);
typedef int32_t (ABI *AttrInit)(void **);
typedef int32_t (ABI *AttrType)(void **, int);
typedef int32_t (ABI *MutexInit)(void **, void **, const char *);
typedef int32_t (ABI *HandleOp)(void **);
typedef int32_t (ABI *Allocate)(int64_t, int64_t, uint64_t, uint64_t, int, int64_t *);
typedef int32_t (ABI *Map)(void **, uint64_t, int, int, int64_t, uint64_t);
typedef int32_t (ABI *Unmap)(void *, uint64_t);
typedef int32_t (ABI *Release)(uint64_t, uint64_t);
typedef void *(ABI *TlsAddress)(const uint64_t *);
typedef void *(ABI *ThreadSelf)(void);
typedef int32_t (ABI *ThreadGetAttr)(void *, void **);
typedef int32_t (ABI *ThreadAffinity)(void **, uint64_t *);
typedef struct { int64_t seconds, nanoseconds; } TestTime;
typedef int32_t (ABI *TimedLock)(void **, const TestTime *);
#define GET(type, name) ((type)runtime_resolve(name, 0))
