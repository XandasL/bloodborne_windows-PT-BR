#pragma once
#include "test_runtime_types.h"

static HandleOp rw_read, rw_write, rw_tryread, rw_trywrite, rw_unlock, rw_destroy;
static void rw_setup(void) {
    rw_read=GET(HandleOp,"Ox9i0c7L5w0#p#J"); rw_write=GET(HandleOp,"mqdNorrB+gI#p#J");
    rw_tryread=GET(HandleOp,"XD3mDeybCnk#p#J"); rw_trywrite=GET(HandleOp,"bIHoZCTomsI#p#J");
    rw_unlock=GET(HandleOp,"+L98PIbGttk#p#J"); rw_destroy=GET(HandleOp,"BB+kb08Tl9A#p#J");
    assert(rw_read && rw_write && rw_tryread && rw_trywrite && rw_unlock && rw_destroy);
}

static void rw_lifecycle(void) {
    rw_setup();
    MutexInit init=GET(MutexInit,"6ULAa0fq4jA#p#J");
    void *lock;
    assert((uint32_t)init(NULL,NULL,NULL)==0x80020016);
    assert(init(&lock,NULL,"rw-test")==0);
    assert(rw_read(&lock)==0 && rw_read(&lock)==0);
    assert((uint32_t)rw_trywrite(&lock)==0x80020010 && (uint32_t)rw_write(&lock)==0x8002000b);
    assert((uint32_t)rw_destroy(&lock)==0x80020010);
    assert(rw_unlock(&lock)==0 && rw_unlock(&lock)==0 && (uint32_t)rw_unlock(&lock)==0x80020001);
    assert(rw_write(&lock)==0 && (uint32_t)rw_tryread(&lock)==0x80020010);
    assert((uint32_t)rw_read(&lock)==0x8002000b && rw_unlock(&lock)==0 && rw_destroy(&lock)==0);
    assert((uintptr_t)lock==1 && (uint32_t)rw_read(&lock)==0x80020016 && (uint32_t)rw_destroy(&lock)==0x80020016);
    lock=NULL;
    assert(rw_destroy(&lock)==0 && rw_read(&lock)==0 && rw_unlock(&lock)==0 && rw_destroy(&lock)==0);
    lock=(void *)(uintptr_t)0xdead;
    assert((uint32_t)rw_tryread(&lock)==0x80020016);
}

typedef struct {
    void *lock;
    pthread_barrier_t ready, release, writer_ready;
    _Atomic int value;
} RwFixture;

static void barrier(pthread_barrier_t *b) {
    int e=pthread_barrier_wait(b); assert(e==0 || e==PTHREAD_BARRIER_SERIAL_THREAD);
}

static void fixture_init(RwFixture *f) {
    assert(pthread_barrier_init(&f->ready,NULL,2)==0);
    assert(pthread_barrier_init(&f->release,NULL,2)==0);
    assert(pthread_barrier_init(&f->writer_ready,NULL,2)==0);
}

static void fixture_destroy(RwFixture *f) {
    assert(rw_destroy(&f->lock)==0);
    assert(pthread_barrier_destroy(&f->ready)==0);
    assert(pthread_barrier_destroy(&f->release)==0);
    assert(pthread_barrier_destroy(&f->writer_ready)==0);
}
