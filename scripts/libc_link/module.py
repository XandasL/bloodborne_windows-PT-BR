# SPDX-License-Identifier: GPL-2.0-or-later
"""ELF/SELF module parsing and symbol identity extraction."""

import hashlib
from prepare import parse_self, span, unpack
from .encode import encode_id


def parse_symbols(blob, tags, str_fn, id_fn):
    symbols, table = [], span(blob, tags[0x61000039], tags[0x6100003f])
    for pos in range(0, len(table), 24):
        n, info, _, sec, val, sz = unpack('<IBBHQQ', table, pos)
        name = str_fn(n)
        symbols.append(dict(name=name, type=info & 15, binding=info >> 4,
                            section=sec, value=val, size=sz, identity=id_fn(name) if name else None))
    return symbols


def parse_relocs(blob, tags):
    relocs = []
    for ot, st in ((0x61000029, 0x6100002d), (0x6100002f, 0x61000031)):
        table = span(blob, tags[ot], tags[st])
        for pos in range(0, len(table), 24):
            target, info, addend = unpack('<QQq', table, pos)
            relocs.append((target, info & 0xffffffff, info >> 32, addend))
    return relocs


def module(path):

    """Parse a game or system module."""
    source = path.read_bytes()
    elf, header, ph, _, missing = parse_self(source)
    dp = next(p for p in ph if p['type'] == 2)
    dynamic = []
    for pos in range(dp['offset'], dp['offset'] + dp['filesz'], 16):
        tag, value = unpack('<QQ', elf, pos)
        if not tag:
            break
        dynamic.append((tag, value))
    tags = dict(dynamic)
    lp = next(p for p in ph if p['type'] == 0x61000000)
    blob = span(elf, lp['offset'], lp['filesz'])
    strings = span(blob, tags[0x61000035], tags[0x61000037])

    def string(offset):
        span(strings, offset, 1)
        return strings[offset:strings.index(0, offset)].decode('ascii')

    libraries, modules = {}, {}
    for tag, value in dynamic:
        if tag in (0x61000013, 0x61000015, 0x6100000d, 0x6100000f):
            tbl = libraries if tag in (0x61000013, 0x61000015) else modules
            key = encode_id(value >> 48)
            ident = (string(value & 0xffffffff), (value >> 32) & 65535)
            if key in tbl and tbl[key] != ident:
                raise ValueError('conflicting module/library IDs')
            tbl[key] = ident

    def identity(name):
        parts = name.split('#')
        if len(parts) != 3:
            raise ValueError(f'unsupported symbol encoding {name!r}')
        n, lib, mod = parts
        return (n, libraries[lib], modules[mod])

    symbols = parse_symbols(blob, tags, string, identity)
    relocs = parse_relocs(blob, tags)
    return dict(elf=elf, header=header, ph=ph, tags=tags, symbols=symbols,
                relocs=relocs, identity=identity, libraries=libraries,
                modules=modules, sha256=hashlib.sha256(source).hexdigest(),
                missing=missing)
