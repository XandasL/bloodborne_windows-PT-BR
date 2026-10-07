# SPDX-License-Identifier: GPL-2.0-or-later
"""Relocation and export mapping for libc_link."""

import struct


def apply_libc_relocations(libc, base, mapped, relocs, imported, image):
    """Process libc relocation entries."""
    tls_count = 0
    for target, kind, symid, addend in libc['relocs']:
        if not mapped(target):
            raise ValueError(f'libc relocation outside image {target:#x}')
        target += base
        if kind == 8:
            relocs.append((target, 0, base + addend, 0))
        elif kind == 16:
            symbol = libc['symbols'][symid]
            if symbol['name'] or symbol['type'] != 3 or addend:
                raise ValueError('unsupported external TLS relocation')
            struct.pack_into('<Q', image, target, 2)
            tls_count += 1
        elif kind in (1, 6, 7):
            symbol = libc['symbols'][symid]
            if symbol['section']:
                if symbol['section'] == 0xfff1:
                    struct.pack_into('<Q', image, target, symbol['value'] + addend)
                else:
                    relocs.append((target, 0, base + symbol['value'] + addend, 0))
            else:
                rk = 2 if symbol['type'] == 1 else 1
                if (rk == 1 and addend) or not 0 <= addend < 4096:
                    raise ValueError('unsupported libc import addend')
                relocs.append((target, rk, imported(symbol), addend))
        else:
            raise ValueError(f'unsupported libc relocation type {kind}')
    return tls_count


def map_exports(libc, mapped, identities, base):
    """Map exports to identities."""
    exports = {}
    for s in libc['symbols']:
        if s['section'] and s['binding'] in (1, 2) and s['identity'] and s['type'] in (1, 2):
            if not mapped(s['value'], max(1, s['size'])):
                raise ValueError('export outside libc load segments')
            if s['identity'] in exports:
                raise ValueError('ambiguous libc export')
            exports[s['identity']] = s
    bindings = []
    for identity, index in identities.items():
        if identity in exports:
            s = exports[identity]
            bindings.append((index, base + s['value'], 2 if s['type'] == 1 else 1))
    return bindings
