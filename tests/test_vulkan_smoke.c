/* SPDX-License-Identifier: MIT
 * Host Vulkan Hardware Smoke Test Runner.
 */
#include <stdio.h>

int vulkan_smoke(void);

int main(void) {
    printf("[TEST] Launching Vulkan Smoke Test on host GPU...\n");
    int res = vulkan_smoke();
    printf("[TEST] Vulkan Smoke Test Result: %s (code %d)\n", res == 0 ? "SUCCESS / PASS" : "FAILED", res);
    return res;
}
