/* SPDX-License-Identifier: MIT
 * PS4 AppContent Provider Implementation.
 * Single responsibility: Offline base-game profile queries and parameter lookup. (~65 LOC)
 */
#include "r_content_types.h"

void require_provider(void) {
    if (!g_content_configured) { fputs("STOP: AppContent needs --content-profile\n", stderr); exit(21); }
}

void require_loaded(void) {
    require_provider();
    if (!g_content_refs) { fputs("STOP: AppContent called before module load\n", stderr); exit(21); }
}

void require_initialized(void) {
    require_loaded();
    if (!g_content_initialized) { fputs("STOP: AppContent query before initialization\n", stderr); exit(21); }
}

void runtime_content_configure(const uint32_t values[5]) {
    if (g_content_refs || g_content_initialized || (values[0] != 1 && values[0] != 3)) {
        fputs("ERROR: invalid or late content profile\n", stderr); exit(1);
    }
    memcpy(g_content_parameters, values, sizeof(g_content_parameters));
    g_content_configured = 1;
}

ABI int32_t content_init(const unsigned char *init, unsigned char *boot) {
    require_loaded();
    if (!init || !boot) return CONTENT_PARAMETER;
    if (g_content_initialized) return CONTENT_BUSY;
    for (unsigned i = 0; i < 32; ++i) if (init[i]) return CONTENT_PARAMETER;
    memset(boot, 0, 40);
    g_content_initialized = 1;
    puts("Runtime: AppContent initialized; offline base-game profile, no mounted add-ons");
    return 0;
}

ABI int32_t content_param_int(uint32_t id, int32_t *out) {
    require_initialized();
    if (id > 4 || !out) return CONTENT_PARAMETER;
    memcpy(out, &g_content_parameters[id], 4);
    ++g_content_queries;
    printf("Runtime: AppContent parameter %u = %d\n", id, *out);
    return 0;
}

ABI int32_t content_addon_list(uint32_t service, void *list, uint32_t capacity, uint32_t *hits) {
    require_initialized();
    if (service) { fputs("STOP: nonzero AppContent service label unsupported\n", stderr); exit(21); }
    if ((!capacity || !list) && !hits) return CONTENT_PARAMETER;
    if (hits) *hits = 0;
    ++g_content_lists;
    return 0;
}
