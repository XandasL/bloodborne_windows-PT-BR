# SPDX-License-Identifier: GPL-2.0-or-later
"""Multi-module linker coordination for guest modules."""

import collections
from libc_link.setup_boot import setup_raw_boot
from .constants import DEFAULT_MODULES
from .load_module import link_one_module
from .module_parser import module
from .patcher import patch_fs_loads
from .relocs import resolve_bindings
from .writer import write_boot_linked, write_report


def link(game, out, module_names=DEFAULT_MODULES):
    """Link eboot with guest modules."""
    main = module(game / 'eboot.bin')
    raw = (out / 'boot.bin').read_bytes()
    hdr, segments, names, relocs, image = setup_raw_boot(raw)
    size, entry, flags = hdr[1], hdr[2], hdr[6]
    identities = {main['identity'](n): i for i, n in enumerate(names)}
    main_libs = {id_v: k for k, id_v in main['libraries'].items()}
    main_mods = {id_v: k for k, id_v in main['modules'].items()}

    def imported(sym):
        k = sym['identity']
        if k not in identities:
            n, lib, mod = k
            name = (f'{n}#{main_libs[lib]}#{main_mods[mod]}'
                    if lib in main_libs and mod in main_mods else f'{n}#{lib[0]}')
            identities[k] = len(names)
            names.append(name)
        return identities[k]

    exports, by_nid, table = {}, collections.defaultdict(list), []
    base, fs_patched, tls_mod = (size + 65535) & ~65535, patch_fs_loads(image, main['ph'], 0), 2
    for filename in module_names:
        m = module(game / 'sce_module' / filename)
        modsz, tls_mod = link_one_module(m, filename, base, tls_mod, imported, relocs,
                                         image, segments, exports, by_nid, table)
        fs_patched += patch_fs_loads(image, m['ph'], base)
        base = (base + modsz + 65535) & ~65535

    bindings, unresolved = resolve_bindings(identities, exports, by_nid, names)
    main_tls = next((p for p in main['ph'] if p['type'] == 7), None)
    main_tls_vals = (main_tls['vaddr'], main_tls['filesz'], main_tls['memsz'], main_tls['align']) if main_tls else (0, 0, 0, 0)
    procparam = next(p for p in main['ph'] if p['type'] == 0x61000001)
    write_boot_linked(out, image, entry, segments, relocs, names, flags, procparam, main_tls_vals, table, bindings)
    write_report(out, table, bindings, names, fs_patched, main_tls_vals, unresolved)
    summary = ', '.join(f"{t['file']}@{t['base']:#x}" for t in table)
    print(f'Linked modules: {summary}; {len(bindings)} native bindings, {len(unresolved)} imports left, fs->gs patched={fs_patched}')
