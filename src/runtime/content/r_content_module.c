/* SPDX-License-Identifier: MIT
 * PS4 System Module Tracker.
 * Single responsibility: Reference counting for system libraries. (~50 LOC)
 */
#include "r_content_types.h"

unsigned g_content_refs = 0;
unsigned g_content_initialized = 0;
unsigned g_content_configured = 0;
uint32_t g_content_parameters[5] = {0};
unsigned g_content_queries = 0;
unsigned g_content_lists = 0;

static unsigned module_refs[0x10000];

ABI int32_t content_module_load(uint16_t id) {
    if (!id) return CONTENT_INVALID_ID;
    if (id == 0xb4) require_provider();
    if (module_refs[id] == UINT32_MAX) { fputs("STOP: module reference overflow\n", stderr); exit(21); }
    ++module_refs[id];
    if (id == 0xb4) {
        g_content_refs = module_refs[id];
        printf("Runtime: AppContent host provider loaded; references=%u\n", g_content_refs);
    } else {
        printf("Runtime: system module 0x%x loaded (host implementation)\n", id);
    }
    return 0;
}

ABI int32_t content_module_loaded(uint16_t id) {
    if (!id) return CONTENT_INVALID_ID;
    return module_refs[id] ? 0 : CONTENT_NOT_LOADED;
}

ABI int32_t content_module_unload(uint16_t id) {
    int32_t result = content_module_loaded(id);
    if (result) return result;
    --module_refs[id];
    if (id == 0xb4) {
        g_content_refs = module_refs[id];
        if (!g_content_refs) g_content_initialized = 0;
    }
    return 0;
}
