#include "test_runtime_types.h"
#include "test_runtime_libc.h"
#include "test_runtime_mutex.h"
#include "test_runtime_mem.h"
#include "test_runtime_rwlock.h"
#include "test_runtime_rwlock_concurrency.h"

int main(int argc, char **argv) {
    runtime_start(0); assert(runtime_resolve("bzQExy189ZI#q#q",0)==0);
    runtime_start(1);
    assert(runtime_resolve("bzQExy189ZI#wrong#module",0)==0);
    assert(runtime_resolve("unknown",0)==0);
    assert(runtime_resolve("8zTFvBIAIN8#q#q",1)==0);
    assert(runtime_resolve("f7uOxY9mM1U#p#J",0)==0);
    uint64_t *canary=(uint64_t *)runtime_resolve("f7uOxY9mM1U#p#J",1);
    assert(canary && *canary && !(*canary & 255));
    if (argc>1 && !strcmp(argv[1],"--guard-recursion")) {
        uint64_t guard=0; Acquire acquire=GET(Acquire,"3GPpjQdAMTw#q#q");
        acquire(&guard); acquire(&guard); return 99;
    }
    if (argc>1 && !strcmp(argv[1],"--bad-tls")) {
        static const char data[4]={0};
        runtime_set_libc_tls(data,4,8);
        const uint64_t index[]={2,8};
        GET(TlsAddress,"vNe1w4diLCs#p#J")(index); return 99;
    }
    if (argc>1 && !strcmp(argv[1],"--stack-failure")) {
        GET(GuestCallback,"Ou3iL1abvng#p#J")(); return 99;
    }
    if (argc>1 && !strcmp(argv[1],"--rwlock-lifecycle")) { rw_lifecycle(); return 0; }
    if (argc>1 && !strcmp(argv[1],"--rwlock-concurrency")) { rw_concurrency(); return 0; }
    if (argc>1 && !strcmp(argv[1],"--rwlock-timeouts")) { rw_timeouts(); return 0; }
    libc_support(); posix_mutexes(); wall_time(); exit_handlers(); guards(); mutexes(); direct_memory(); memory_primitives();
    rw_lifecycle(); rw_concurrency(); rw_timeouts();
    puts("PASS: callback lifecycle, guard ABI, mutex errors, shared direct memory, memory primitives, resolver scope");
    return 0;
}
