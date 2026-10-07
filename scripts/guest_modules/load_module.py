# SPDX-License-Identifier: GPL-2.0-or-later
"""Single module loading helper for guest module linker."""

from prepare import span
from .relocs import apply_module_relocs, collect_module_exports


def link_one_module(m, filename, base, tls_mod, imported, relocs,
                    image, segments, exports, by_nid, table):
    """Load and process a single guest module."""
    loads = [p for p in m['ph'] if p['type'] in (1, 0x61000010)]
    modsize = max(p['vaddr'] + p['memsz'] for p in loads)
    if base + modsize > 512 * 1024 * 1024:
        raise ValueError('linked image exceeds probe limit')
    image.extend(bytes(base + modsize - len(image)))

    def mapped(addr, sz=8):
        return any(p['vaddr'] <= addr and addr + sz <= p['vaddr'] + p['memsz'] for p in loads)

    for p in loads:
        if p['filesz'] > p['memsz']:
            raise ValueError(f'{filename}: invalid segment size')
        image[base + p['vaddr']:base + p['vaddr'] + p['filesz']] = span(m['elf'], p['offset'], p['filesz'])
        segments.append((base + p['vaddr'], p['memsz'], p['flags']))
    tls = next((p for p in m['ph'] if p['type'] == 7), None)
    mod_id = 0
    if tls:
        if tls['filesz'] > tls['memsz'] or not mapped(tls['vaddr'], tls['memsz']):
            raise ValueError(f'{filename}: unsupported TLS layout')
        mod_id = tls_mod
        tls_mod += 1
    tls_relocs = apply_module_relocs(m, filename, base, mod_id, mapped, imported, relocs, image)
    exp_count = collect_module_exports(m, filename, base, mapped, exports, by_nid)
    table.append(dict(file=filename, base=base, size=modsize, init=base + m['tags'].get(12, 0),
                      tls_address=base + tls['vaddr'] if tls else 0, tls_memsz=tls['memsz'] if tls else 0,
                      tls_filesz=tls['filesz'] if tls else 0, tls_module=mod_id,
                      tls_relocations=tls_relocs, exports=exp_count, sha256=m['sha256']))
    return modsize, tls_mod
