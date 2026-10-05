/* SPDX-License-Identifier: MIT
 * Windows Platform Vectored Exception Handling (VEH) Test.
 */
#include "platform/faults.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <assert.h>

static void* g_protected_page = NULL;
static volatile int g_gpu_fault_handled = 0;

static int gpu_fault_test_handler(void* context, void* fault_addr) {
    (void)context;
    if (fault_addr == g_protected_page) {
        printf("  [PASS] VEH intercepted GPU page fault at address %p\n", fault_addr);
        // Repair page: change to PAGE_READWRITE
        DWORD old_prot;
        VirtualProtect(g_protected_page, 4096, PAGE_READWRITE, &old_prot);
        g_gpu_fault_handled = 1;
        return 1; // EXCEPTION_CONTINUE_EXECUTION
    }
    return 0; // Not our page
}

int main(void) {
    printf("[TEST] Starting Windows Vectored Exception Handler (VEH) Test...\n");

    // Initialize VEH
    bb_platform_faults_init();
    bb_platform_faults_register_gpu(gpu_fault_test_handler);
    printf("  [PASS] Vectored exception handler registered\n");

    // 1. Test GPU Dirty-Page Tracking (Fault -> Repair -> Continue Execution)
    g_protected_page = VirtualAlloc(NULL, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_NOACCESS);
    assert(g_protected_page != NULL);

    printf("  [TEST] Attempting write to PAGE_NOACCESS at %p...\n", g_protected_page);
    volatile uint32_t* ptr = (volatile uint32_t*)g_protected_page;

    // This write triggers EXCEPTION_ACCESS_VIOLATION, VEH catches it, repairs to PAGE_READWRITE, resumes execution
    *ptr = 0x1337BEEF;

    assert(g_gpu_fault_handled == 1);
    assert(*ptr == 0x1337BEEF);
    printf("  [PASS] Thread successfully resumed execution: value 0x%X read back\n", *ptr);

    VirtualFree(g_protected_page, 0, MEM_RELEASE);

    // 2. Test Recovery Point (Fault -> longjmp Recovery)
    jmp_buf recover_buf;
    bb_platform_faults_set_recovery(&recover_buf);

    volatile int recovered = 0;
    if (setjmp(recover_buf) == 0) {
        printf("  [TEST] Triggering unhandled access violation to test recovery jump...\n", (void*)0);
        volatile int* crash_ptr = (volatile int*)0x42;
        *crash_ptr = 999; // triggers VEH -> longjmp
        assert(0); // should not reach here
    } else {
        recovered = 1;
        printf("  [PASS] Successfully recovered from access violation via longjmp\n");
    }
    assert(recovered == 1);

    printf("[SUCCESS] Windows VEH test PASSED!\n");
    return 0;
}
