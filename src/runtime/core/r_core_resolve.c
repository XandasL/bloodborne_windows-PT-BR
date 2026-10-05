/* SPDX-License-Identifier: MIT
 * PS4 Core Symbol Resolution Table.
 * Single responsibility: NID to symbol name dictionary lookup. (~45 LOC)
 */
#include "r_core_types.h"

static const struct { const char *nid, *symbol; } import_names[] = {
#include "import_names.inc"
};

const char *runtime_symbol(const char *nid) {
    for (size_t i = 0; i < sizeof(import_names) / sizeof(*import_names); ++i)
        if (!strcmp(nid, import_names[i].nid)) return import_names[i].symbol;
    return NULL;
}

uintptr_t runtime_lookup(const RuntimeExport *table, size_t count, const char *nid) {
    const char *symbol = runtime_symbol(nid);
    if (!symbol) return 0;
    for (size_t i = 0; i < count; ++i)
        if (!strcmp(symbol, table[i].name)) return (uintptr_t)table[i].function;
    return 0;
}

const char *runtime_import_name(const char *name) {
    const char *symbol = runtime_symbol(name);
    return symbol ? symbol : "name not resolved; see analysis.json import_name_hints";
}
