/* SPDX-License-Identifier: MIT
 * PS4 AppContent Exports and Reporting.
 * Single responsibility: Export table and diagnostic stats. (~40 LOC)
 */
#include "r_content_types.h"

ABI int32_t content_module_load(uint16_t id);
ABI int32_t content_module_loaded(uint16_t id);
ABI int32_t content_module_unload(uint16_t id);
ABI int32_t content_init(const unsigned char *init, unsigned char *boot);
ABI int32_t content_param_int(uint32_t id, int32_t *out);
ABI int32_t content_addon_list(uint32_t service, void *list, uint32_t capacity, uint32_t *hits);

uintptr_t runtime_content_resolve(const char *name) {
    if (!strcmp(name, "g8cM39EUZ6o#M#N")) return (uintptr_t)content_module_load;
    if (!strcmp(name, "fMP5NHUOaMk#M#N")) return (uintptr_t)content_module_loaded;
    if (!strcmp(name, "eR2bZFAAU0Q#M#N")) return (uintptr_t)content_module_unload;
    if (!strcmp(name, "R9lA82OraNs#c#d")) return (uintptr_t)content_init;
    if (!strcmp(name, "99b82IKXpH4#c#d")) return (uintptr_t)content_param_int;
    if (!strcmp(name, "xnd8BJzAxmk#c#d")) return (uintptr_t)content_addon_list;
    return 0;
}

void runtime_content_report(void) {
    printf("Runtime: AppContent references=%u, initialized=%u, parameter queries=%u, list queries=%u\n",
           g_content_refs, g_content_initialized, g_content_queries, g_content_lists);
}
