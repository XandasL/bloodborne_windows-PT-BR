#pragma once
#include "test_runtime_rwlock.h"

static void *reader_worker(void *context) {
    RwFixture *f=context;
    assert(rw_read(&f->lock)==0);
    barrier(&f->ready); barrier(&f->release);
    assert(atomic_load(&f->value)==0 && rw_unlock(&f->lock)==0);
    return NULL;
}

static void *writer_worker(void *context) {
    RwFixture *f=context;
    assert((uint32_t)rw_trywrite(&f->lock)==0x80020010);
    barrier(&f->writer_ready);
    assert(rw_write(&f->lock)==0);
    atomic_store(&f->value,1);
    assert(rw_unlock(&f->lock)==0);
    return NULL;
}

static void *holding_writer(void *context) {
    RwFixture *f=context;
    assert(rw_write(&f->lock)==0);
    barrier(&f->ready); barrier(&f->release);
    assert(rw_unlock(&f->lock)==0);
    return NULL;
}

static void rw_concurrency(void) {
    rw_setup();
    RwFixture f={0}; fixture_init(&f);
    pthread_t reader, writer;
    assert(rw_read(&f.lock)==0 && pthread_create(&reader,NULL,reader_worker,&f)==0);
    barrier(&f.ready);
    assert(rw_unlock(&f.lock)==0 && pthread_create(&writer,NULL,writer_worker,&f)==0);
    barrier(&f.writer_ready);
    assert((uint32_t)rw_destroy(&f.lock)==0x80020010 && atomic_load(&f.value)==0);
    barrier(&f.release);
    assert(pthread_join(reader,NULL)==0 && pthread_join(writer,NULL)==0 && atomic_load(&f.value)==1);
    fixture_destroy(&f);
}

static void rw_timeouts(void) {
    rw_setup();
    TimedLock read_timed=GET(TimedLock,"iPtZRWICjrM#p#J"), write_timed=GET(TimedLock,"adh--6nIqTk#p#J");
    RwFixture f={0}; fixture_init(&f);
    pthread_t writer; assert(pthread_create(&writer,NULL,holding_writer,&f)==0);
    barrier(&f.ready);
    TestTime expired={0,0}, invalid={0,1000000000};
    assert((uint32_t)read_timed(&f.lock,&expired)==0x8002003c);
    assert((uint32_t)write_timed(&f.lock,&expired)==0x8002003c);
    assert((uint32_t)read_timed(&f.lock,&invalid)==0x80020016);
    assert((uint32_t)rw_unlock(&f.lock)==0x80020001);
    barrier(&f.release); assert(pthread_join(writer,NULL)==0);
    assert(read_timed(&f.lock,&invalid)==0 && rw_unlock(&f.lock)==0);
    fixture_destroy(&f);
}
