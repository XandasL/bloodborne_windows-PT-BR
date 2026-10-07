/* SPDX-License-Identifier: MIT
 * PS4 Loader Relocation Application.
 * Single responsibility: Resolve HLE/native imports and apply relocation writes.
 */
#include "loader_types.h"

void loader_apply_relocations(Segment *segments, uint64_t ns, Reloc *relocs, uint64_t nr,
                              unsigned char *traps, const uint64_t *bindings,
                              const uint64_t *binding_kinds, int native_libc) {
    size_t data_bytes = (g_loader_import_count + 1) * g_page_size;
    unsigned char *data_traps = (unsigned char*)loader_allocate(data_bytes);
    loader_protect(data_traps, data_bytes, 0);

    for (uint64_t i = 0; i < nr; ++i) {
        Reloc *r = &relocs[i];
        uintptr_t value = r->kind == 2
            ? (uintptr_t)(data_traps + g_page_size * r->value + r->addend)
            : r->kind == 1
                ? (uintptr_t)(traps + 32 * r->value)
                : (uintptr_t)g_loader_image + r->value;
        if (r->kind) {
            uintptr_t resolved = runtime_resolve(g_loader_names[r->value], r->kind == 2);
            if (resolved) {
                value = resolved + r->addend;
            } else if (native_libc && bindings[r->value]) {
                uint64_t address = bindings[r->value];
                if (binding_kinds[r->value] != r->kind ||
                    !loader_mapped(segments, ns, address, r->addend + 1))
                    fail("native export kind/range mismatch");
                value = (uintptr_t)g_loader_image + address + r->addend;
            }
        }
        memcpy(g_loader_image + r->target, &value, 8);
    }
}
