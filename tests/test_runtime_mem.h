#pragma once
#include "test_runtime_types.h"

static void wall_time(void) {
    typedef int (ABI *GetTime)(int64_t *,int32_t *);
    GetTime get=GET(GetTime,"n88vx3C5nW8#I#J");
    struct { int64_t value[2]; uint64_t canary; } t={{0,0},0xabcdef};
    struct { int32_t value[2]; uint32_t canary; } z={{0,0},0x123456};
    time_t before=time(NULL);
    assert(get(t.value,z.value)==0);
    time_t after=time(NULL);
    assert(t.value[0]>=before && t.value[0]<=after && t.value[1]>=0 && t.value[1]<1000000);
    assert(t.canary==0xabcdef && z.canary==0x123456);
    assert(get(NULL,NULL)==0 && get(NULL,z.value)==0 && !runtime_resolve("n88vx3C5nW8#q#q",0));
}

static void direct_memory(void) {
    Allocate alloc=GET(Allocate,"rTXw65xmLIA#p#J");
    Map map=GET(Map,"L-Q3LEjIbgA#p#J");
    Unmap unmap=GET(Unmap,"cQke9UuBQOk#p#J");
    Release release=GET(Release,"MBuItvba6z8#p#J");
    int64_t a=-1, b=-1, c=-1;
    const uint64_t pool=536870912, length=16384;
    assert((uint32_t)alloc(0,pool,1,0,0,&a)==0x80020016 && a==-1);
    assert((uint32_t)alloc(0,8192,length,0,0,&a)==0x80020023);
    assert(alloc(0,pool,length,2097152,0,&a)==0 && a==0);
    assert(alloc(0,pool,length,2097152,0,&b)==0 && b==2097152);
    assert(alloc(0,pool,length,16384,0,&c)==0 && c==16384);
    void *x=NULL, *y=NULL, *invalid=NULL;
    assert((uint32_t)map(&invalid,length*2,3,0,b,0)==0x80020016);
    assert(map(&x,length,3,0,a,2097152)==0 && !((uintptr_t)x%2097152));
    assert(map(&y,length,3,0,a,0)==0 && *(uint64_t *)x==0);
    *(uint64_t *)x=UINT64_C(0xabcdef0123456789);
    assert(*(uint64_t *)y==UINT64_C(0xabcdef0123456789));
    assert((uint32_t)unmap((char *)x+4096,length)==0x80020016);
    assert(unmap(x,length)==0 && unmap(y,length)==0);
    assert(release(a,length)==0 && release(b,length)==0 && release(c,length)==0);
    assert(alloc(0,pool,length,0,0,&a)==0 && a==0);
    x=NULL; assert(map(&x,length,3,0,a,0)==0 && *(uint64_t *)x==0 && release(a,length)==0);
}

static void memory_primitives(void) {
    typedef void *(ABI *Set)(void *,int,size_t);
    typedef void *(ABI *Copy)(void *,const void *,size_t);
    typedef int (ABI *Compare)(const void *,const void *,size_t);
    char a[16], b[16];
    assert(GET(Set,"8zTFvBIAIN8#q#q")(a,0x5a,sizeof(a))==a);
    assert(GET(Copy,"Q3VBxCXhUHs#q#q")(b,a,sizeof(a))==b);
    assert(GET(Compare,"DfivPArhucg#q#q")(a,b,sizeof(a))==0);
    memcpy(a,"abcdef",7);
    GET(Copy,"+P6FRGH4LfA#q#q")(a+1,a,6);
    assert(memcmp(a,"aabcdef",7)==0);
    typedef size_t (ABI *Length)(const char *);
    Length length=GET(Length,"j4ViWNHEgww#q#q");
    assert(length("")==0 && length("abc\0def")==3 && length("\xff\x80")==2);
}
