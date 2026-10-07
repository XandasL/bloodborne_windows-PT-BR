# SPDX-License-Identifier: GPL-2.0-or-later
"""ELF dynamic relocation and symbol table processing."""

import collections
import struct
from .binary_utils import span, unpack


def process_symbols(blob, tags):
    """Extract symbol entries from dynamic symbol table."""
    strings = span(blob, tags[0x61000035], tags[0x61000037])

    def string(off):
        span(strings, off, 1)
        return strings[off:strings.index(0, off)].decode('ascii')

    syms = span(blob, tags[0x61000039], tags[0x6100003f])
    if len(syms) % 24:
        raise ValueError('bad symbol table size')
    symbols = []
    for pos in range(0, len(syms), 24):
        name, info, _, shndx, value, sz = unpack('<IBBHQQ', syms, pos)
        symbols.append(dict(name=string(name), type=info & 15, section=shndx, value=value, size=sz))
    return symbols


def process_relocs(blob, tags, symbols, mapped, image):
    """Process ELF relocations into compact boot structures."""
    relocs, counts, imports = [], collections.Counter(), {}
    for offset_tag, size_tag in ((0x61000029, 0x6100002d), (0x6100002f, 0x61000031)):
        table = span(blob, tags[offset_tag], tags[size_tag])
        if len(table) % 24:
            raise ValueError('bad relocation table size')
        for pos in range(0, len(table), 24):
            target, info, addend = unpack('<QQq', table, pos)
            kind, sym = info & 0xffffffff, info >> 32
            counts[kind] += 1
            if not mapped(target):
                raise ValueError(f'relocation outside mapped segments: {target:#x}')
            if kind == 8:
                relocs.append((target, 0, addend, 0))
            elif kind in (1, 6, 7):
                symbol = symbols[sym]
                if symbol['section']:
                    if symbol['section'] == 0xfff1:
                        struct.pack_into('<Q', image, target, symbol['value'] + addend)
                    else:
                        relocs.append((target, 0, symbol['value'] + addend, 0))
                else:
                    if sym not in imports:
                        imports[sym] = len(imports)
                    k = 2 if symbol['type'] == 1 else 1
                    if (k == 1 and addend) or not 0 <= addend < 4096:
                        raise ValueError('unsupported import addend')
                    relocs.append((target, k, imports[sym], addend))
            else:
                raise ValueError(f'unsupported relocation {kind}')
    return relocs, counts, imports
