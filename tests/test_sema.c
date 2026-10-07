#include "test_sema_ops.h"
#include "test_sema_lifecycle.h"
#include "test_sema_concurrency.h"

int main(int argc, char **argv) {
    setup();
    if (argc > 1 && !strcmp(argv[1], "--priority")) {
        uint32_t id; assert(create(&id, "priority", 2, 0, 1, NULL) == 0);
        Task a = {id, 1, 99, 5000000}; pthread_t t;
        assert(pthread_create(&t, NULL, waiter, &a) == 0); enrolled(id, 1);
        assert(signal_sem(id, 1) == 0);
        assert(pthread_join(t, NULL) == 0 && a.result == 0);
        puts("PASS: priority semaphore wait"); return 0;
    }
    if (argc > 1 && !strcmp(argv[1], "--concurrency")) concurrency();
    else if (argc > 1 && !strcmp(argv[1], "--cancel-delete")) cancellation();
    else lifecycle();
    puts("PASS: semaphore contracts");
    return 0;
}
