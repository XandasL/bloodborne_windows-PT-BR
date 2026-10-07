#pragma once
#include "test_runtime_types.h"

static void guards(void) {
    uint64_t state=0;
    Acquire acquire=GET(Acquire,"3GPpjQdAMTw#q#q");
    Guard release=GET(Guard,"9rAeANT2tyE#q#q"), abort_guard=GET(Guard,"2emaaluWzUw#q#q");
    assert(acquire(&state)==1 && state==(UINT64_C(1)<<32));
    abort_guard(&state); assert(state==0);
    assert(acquire(&state)==1);
    release(&state); assert(state==1);
    assert(acquire(&state)==0);
}

static void mutexes(void) {
    AttrInit attr_init=GET(AttrInit,"F8bUHwAG284#p#J");
    AttrType type=GET(AttrType,"iMp8QpE+XO4#p#J");
    HandleOp attr_destroy=GET(HandleOp,"smWEktiyyG0#p#J");
    MutexInit init=GET(MutexInit,"cmo1RIYva9o#p#J");
    HandleOp lock=GET(HandleOp,"9UK1vLZQft4#p#J"), unlock=GET(HandleOp,"tn3VlD0hG60#p#J");
    HandleOp trylock=GET(HandleOp,"upoVrzMHFeE#p#J"), destroy=GET(HandleOp,"2Of0f+3mhhE#p#J");
    void *attr=NULL, *mutex=NULL;
    assert((uint32_t)attr_init(NULL)==0x80020016 && attr_init(&attr)==0);
    assert((uint32_t)type(&attr,99)==0x80020016 && type(&attr,2)==0);
    assert(init(&mutex,&attr,"recursive-test")==0 && attr_destroy(&attr)==0 && attr==NULL);
    assert(lock(&mutex)==0 && trylock(&mutex)==0);
    assert((uint32_t)destroy(&mutex)==0x80020010);
    assert(unlock(&mutex)==0 && unlock(&mutex)==0 && destroy(&mutex)==0);
    assert((uint32_t)lock(&mutex)==0x80020016);
    mutex=NULL;
    assert(lock(&mutex)==0 && (uint32_t)lock(&mutex)==0x8002000b);
    assert((uint32_t)trylock(&mutex)==0x80020010 && unlock(&mutex)==0 && destroy(&mutex)==0);
}

static void posix_mutexes(void) {
    AttrInit init=GET(AttrInit,"dQHWEsJtoE4#I#J");
    AttrType type=GET(AttrType,"mDmgMOGVUqg#I#J");
    HandleOp destroy_attr=GET(HandleOp,"HF7lK46xzjY#I#J");
    typedef int32_t (ABI *PosixInit)(void **,void **);
    PosixInit mutex_init=GET(PosixInit,"ttHNfU+qDBU#I#J");
    HandleOp lock=GET(HandleOp,"7H0iTOciTLo#I#J"), unlock=GET(HandleOp,"2Z+PpY6CaJg#I#J");
    HandleOp trylock=GET(HandleOp,"K-jXhbt2gn4#I#J"), destroy=GET(HandleOp,"ltCfaGr2JGE#I#J");
    void *a=NULL, *m=NULL;
    assert(init(NULL)==22 && init(&a)==0);
    assert(type(&a,99)==22 && type(&a,2)==0);
    assert(mutex_init(&m,&a)==0 && destroy_attr(&a)==0);
    assert(lock(&m)==0 && trylock(&m)==0 && destroy(&m)==16);
    assert(unlock(&m)==0 && unlock(&m)==0 && destroy(&m)==0);
    m=NULL;
    assert(lock(&m)==0 && lock(&m)==11 && trylock(&m)==16);
    assert(unlock(&m)==0 && destroy(&m)==0);
    assert(!runtime_resolve("dQHWEsJtoE4#q#q",0));
}
