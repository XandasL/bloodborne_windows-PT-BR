# SPDX-License-Identifier: GPL-2.0-or-later
"""Guest module relocations and export table collation."""

import struct


def apply_module_relocs(m, filename, base, module_id, mapped, imported, relocs, image):
    """Apply relocations from a guest module."""
    tls_relocations = 0
    for target, kind, symid, addend in m['relocs']:
        if not mapped(target):
            raise ValueError(f'{filename}: relocation outside image {target:#x}')
        target += base
        if kind == 8:
            relocs.append((target, 0, base + addend, 0))
        elif kind == 16:
            sym = m['symbols'][symid]
            if sym['name'] or sym['type'] != 3 or addend or not module_id:
                raise ValueError(f'{filename}: unsupported external TLS relocation')
            struct.pack_into('<Q', image, target, module_id)
            tls_relocations += 1
        elif kind in (1, 6, 7):
            sym = m['symbols'][symid]
            if sym['section']:
                if sym['section'] == 0xfff1:
                    struct.pack_into('<Q', image, target, sym['value'] + addend)
                else:
                    relocs.append((target, 0, base + sym['value'] + addend, 0))
            elif not sym['identity'] and sym['binding'] == 2:
                struct.pack_into('<Q', image, target, 0)
            else:
                rk = 2 if sym['type'] == 1 else 1
                if (rk == 1 and addend) or not 0 <= addend < 4096:
                    raise ValueError(f'{filename}: unsupported import addend')
                relocs.append((target, rk, imported(sym), addend))
        else:
            raise ValueError(f'{filename}: unsupported relocation type {kind}')
    return tls_relocations


def collect_module_exports(m, filename, base, mapped, exports, by_nid):
    """Register exports from a guest module."""
    count = 0
    for s in m['symbols']:
        if s['section'] and s['binding'] in (1, 2) and s['identity'] and s['type'] in (1, 2):
            if not mapped(s['value'], max(1, s['size'])):
                raise ValueError(f'{filename}: export outside load segments')
            if s['identity'] in exports:
                raise ValueError(f'{filename}: ambiguous export')
            entry_value = (base + s['value'], 2 if s['type'] == 1 else 1, filename)
            exports[s['identity']] = entry_value
            by_nid[(s['identity'][0], s['identity'][1][0])].append(entry_value)
            count += 1
    return count


def resolve_bindings(identities, exports, by_nid, names):
    """Match identity references against exports and fallback libraries."""
    bindings, unresolved = [], []
    for identity, index in identities.items():
        found = exports.get(identity)
        if not found and identity and identity[1][0] == 'libSceLibcInternal':
            cands = by_nid.get((identity[0], 'libc'), [])
            found = cands[0] if len(cands) == 1 else None
        if found:
            bindings.append((index, found[0], found[1]))
        else:
            unresolved.append(names[index])
    return bindings, unresolved
