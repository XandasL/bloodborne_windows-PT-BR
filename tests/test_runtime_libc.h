#pragma once
#include "test_runtime_types.h"

static void *main_identity;
static void *tls_worker(void *unused) {
    (void)unused;
    const uint64_t index[]={2,0};
    unsigned char *p=GET(TlsAddress,"vNe1w4diLCs#p#J")(index);
    assert(p[0]==42 && p[1]==17 && p[31]==0);
    p[0]=99;
    assert(GET(ThreadSelf,"aI+OeCz8xrQ#p#J")()!=main_identity);
    return NULL;
}

static void libc_support(void) {
    static const unsigned char initial[]={42,17};
    runtime_set_libc_tls(initial,sizeof(initial),32);
    const uint64_t index[]={2,0}, last[]={2,31};
    TlsAddress tls=GET(TlsAddress,"vNe1w4diLCs#p#J");
    unsigned char *p=tls(index);
    assert(p[0]==42 && p[1]==17 && p[2]==0 && p[31]==0 && tls(last)==p+31);
    p[0]=23; main_identity=GET(ThreadSelf,"aI+OeCz8xrQ#p#J")();
    pthread_t worker;
    assert(pthread_create(&worker,NULL,tls_worker,NULL)==0 && pthread_join(worker,NULL)==0);
    assert(p[0]==23 && initial[0]==42);
    void *attr=NULL;
    AttrInit init=GET(AttrInit,"nsYoNRywwNg#p#J");
    ThreadGetAttr get=GET(ThreadGetAttr,"x1X76arYMxU#p#J");
    ThreadAffinity affinity=GET(ThreadAffinity,"8+s5BzZjxSg#p#J");
    HandleOp destroy=GET(HandleOp,"62KCwEMmzcM#p#J");
    uint64_t mask=0;
    assert(init(&attr)==0 && get(main_identity,&attr)==0 && affinity(&attr,&mask)==0 && mask==0x7f);
    assert(destroy(&attr)==0 && !attr);
    uint64_t param[8]={64}; runtime_set_procparam(param);
    assert(GET(ThreadSelf,"959qrazPIrg#p#J")()==param);
    typedef void (ABI *SetHeap)(void **);
    void *heap[10]={0}; GET(SetHeap,"p5EcQeEeJAE#p#J")(heap);
    assert(runtime_application_heap_api()==heap);
    GET(SetHeap,"p5EcQeEeJAE#p#J")(NULL); runtime_set_procparam(NULL);
}

static int order[512], count;
static Register register_plain;
static ABI void first(void) { order[count++]=1; }
static ABI void second(void) { order[count++]=2; }
static ABI void with_arg(void *p) { order[count++]=(int)(uintptr_t)p; }
static ABI void add_handler(void) { order[count++]=4; assert(register_plain(first)==0); }

static void exit_handlers(void) {
    register_plain=GET(Register,"8G2LB+A3rzg#q#q");
    RegisterCxa cxa=GET(RegisterCxa,"tsvEmnenz48#q#q");
    assert(register_plain(first)==0 && cxa(with_arg,(void *)3,(void *)11)==0 && register_plain(second)==0);
    runtime_finalize((void *)11); assert(count==1 && order[0]==3);
    runtime_finalize(NULL); assert(count==3 && order[1]==2 && order[2]==1);
    assert(register_plain(add_handler)==0); runtime_finalize(NULL);
    assert(count==5 && order[3]==4 && order[4]==1);
    count=0;
    for (uintptr_t i=0; i<200; ++i) assert(cxa(with_arg,(void *)i,(void *)12)==0);
    runtime_finalize((void *)12); assert(count==200);
    for (int i=0; i<200; ++i) assert(order[i]==199-i);
}
