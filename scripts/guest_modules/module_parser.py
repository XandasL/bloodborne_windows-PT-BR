# SPDX-License-Identifier: GPL-2.0-or-later
"""Guest module parser for dynamic tags, symbols, and relocations."""

import hashlib
from link_libc import encode_id
from prepare import parse_self, span, unpack


def parse_symbols(blob, tags, str_fn, id_fn):
    syms, table = [], span(blob, tags[0x61000039], tags[0x6100003f])
    for pos in range(0, len(table), 24):
        n, info, _, sec, val, sz = unpack('<IBBHQQ', table, pos)
        name = str_fn(n)
        syms.append(dict(name=name, type=info & 15, binding=info >> 4,
                         section=sec, value=val, size=sz, identity=id_fn(name) if name else None))
    return syms


def parse_relocs(blob, tags):
    relocs = []
    for ot, st in ((0x61000029, 0x6100002d), (0x6100002f, 0x61000031)):
        table = span(blob, tags[ot], tags[st])
        for pos in range(0, len(table), 24):
            tgt, info, add = unpack('<QQq', table, pos)
            relocs.append((tgt, info & 0xffffffff, info >> 32, add))
    return relocs


def module(path):
    source = path.read_bytes()
    elf, header, ph, _, missing = parse_self(source)
    dp = next(p for p in ph if p['type'] == 2)
    dynamic = []
    for pos in range(dp['offset'], dp['offset'] + dp['filesz'], 16):
        tag, val = unpack('<QQ', elf, pos)
        if not tag:
            break
        dynamic.append((tag, val))
    tags = dict(dynamic)
    lp = next(p for p in ph if p['type'] == 0x61000000)
    blob = span(elf, lp['offset'], lp['filesz'])
    strings = span(blob, tags[0x61000035], tags[0x61000037])

    def string(offset):
        span(strings, offset, 1)
        return strings[offset:strings.index(0, offset)].decode('ascii')

    libraries, modules = {}, {}
    for tag, val in dynamic:
        if tag in (0x61000013, 0x61000015, 0x6100000d, 0x6100000f):
            tbl = libraries if tag in (0x61000013, 0x61000015) else modules
            key = encode_id(val >> 48)
            ident = (string(val & 0xffffffff), (val >> 32) & 65535)
            if key in tbl and tbl[key] != ident:
                raise ValueError('conflicting module/library IDs')
            tbl[key] = ident

    def identity(name):
        parts = name.split('#')
        return (parts[0], libraries[parts[1]], modules[parts[2]]) if len(parts) == 3 else None

    syms = parse_symbols(blob, tags, string, identity)
    relocs = parse_relocs(blob, tags)
    return dict(elf=elf, header=header, ph=ph, tags=tags, symbols=syms, relocs=relocs,
                identity=identity, libraries=libraries, modules=modules,
                sha256=hashlib.sha256(source).hexdigest(), missing=missing)
