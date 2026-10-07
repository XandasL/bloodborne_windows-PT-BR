# SPDX-License-Identifier: GPL-2.0-or-later
"""Main linking routine for libc_link."""

from prepare import span
from .module import module
from .patch_elf import extract_main_tls, patch_fs_to_gs
from .reloc import apply_libc_relocations, map_exports
from .setup_boot import setup_raw_boot
from .writer import write_boot_libc, write_report


def link(game, out):
    """Link libc into game boot image."""
    import sys
    link_libc_mod = sys.modules.get('link_libc')
    mod = getattr(link_libc_mod, 'module', module) if link_libc_mod else module
    main = mod(game / 'eboot.bin')
    libc = mod(game / 'sce_module/libc.prx')

    raw = (out / 'boot.bin').read_bytes()
    hdr, segments, names, relocs, image = setup_raw_boot(raw)
    size, entry, flags = hdr[1], hdr[2], hdr[6]
    base = (size + 65535) & ~65535
    loads = [p for p in libc['ph'] if p['type'] in (1, 0x61000010)]
    libsize = max(p['vaddr'] + p['memsz'] for p in loads)
    if base + libsize > 512 * 1024 * 1024:
        raise ValueError('linked image exceeds probe limit')
    image.extend(bytes(base + libsize - len(image)))

    def mapped(addr, sz=8):
        return any(p['vaddr'] <= addr and addr + sz <= p['vaddr'] + p['memsz'] for p in loads)

    for p in loads:
        if p['filesz'] > p['memsz']:
            raise ValueError('invalid libc segment size')
        image[base + p['vaddr']:base + p['vaddr'] + p['filesz']] = span(libc['elf'], p['offset'], p['filesz'])
        segments.append((base + p['vaddr'], p['memsz'], p['flags']))

    identities = {main['identity'](name): i for i, name in enumerate(names)}
    main_libs = {id_v: k for k, id_v in main['libraries'].items()}
    main_mods = {id_v: k for k, id_v in main['modules'].items()}

    def imported(sym):
        k = sym['identity']
        if k not in identities:
            n, lib, mod = k
            name = (f'{n}#{main_libs[lib]}#{main_mods[mod]}'
                    if lib in main_libs and mod in main_mods else f'libc:{sym["name"]}')
            identities[k] = len(names)
            names.append(name)
        return identities[k]

    tls_count = apply_libc_relocations(libc, base, mapped, relocs, imported, image)
    bindings = map_exports(libc, mapped, identities, base)
    patched = patch_fs_to_gs(image, main['ph'])
    main_tls_values = extract_main_tls(main['ph'], size)
    tls = next(p for p in libc['ph'] if p['type'] == 7)
    if tls['filesz'] > tls['memsz'] or not mapped(tls['vaddr'], tls['memsz']):
        raise ValueError('unsupported TLS layout')
    procparam = next(p for p in main['ph'] if p['type'] == 0x61000001)
    metadata = (base, libsize, base + libc['tags'][12], base + tls['vaddr'],
                tls['memsz'], tls['filesz'], len(bindings), procparam['vaddr'])
    write_boot_libc(out, (b'BBPROBE4', len(image), entry, len(segments), len(relocs), len(names), flags),
                    metadata, main_tls_values, bindings, segments, names, relocs, image)
    write_report(out, libc, base, libsize, metadata, bindings, tls_count, patched, main_tls_values, tls, names)
    print(f'Linked native libc: base={base:#x}, {len(bindings)} fallback exports, TLS={tls["memsz"]} bytes; '
          f'eboot TLS={main_tls_values[2]} bytes, fs->gs patched={patched}')
