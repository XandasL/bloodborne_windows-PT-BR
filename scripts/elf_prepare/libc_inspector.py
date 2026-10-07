# SPDX-License-Identifier: GPL-2.0-or-later
"""Inspection of user plaintext libc _init_env and runtime contracts."""

import hashlib
from .binary_utils import nid, span, unpack
from .self_parser import parse_self


def inspect_libc(path):
    """Prove that this dump's _init_env is exactly RET; never assume it."""
    source = path.read_bytes()
    elf, _, ph, _, _ = parse_self(source)
    dynamic = next(p for p in ph if p['type'] == 2)
    tags = dict(unpack('<QQ', elf, pos) for pos in
                range(dynamic['offset'], dynamic['offset'] + dynamic['filesz'], 16))
    lib = next(p for p in ph if p['type'] == 0x61000000)
    blob = span(elf, lib['offset'], lib['filesz'])
    strings = span(blob, tags[0x61000035], tags[0x61000037])
    syms = span(blob, tags[0x61000039], tags[0x6100003f])
    evidence = {'sha256': hashlib.sha256(source).hexdigest(), 'init_env_is_ret': False}
    for pos in range(0, len(syms), 24):
        name, info, _, section, value, size = unpack('<IBBHQQ', syms, pos)
        span(strings, name, 1)
        symbol = strings[name:strings.index(0, name)].decode('ascii')
        key = symbol.split('#')[0]
        if key not in (nid('_init_env'), nid('_ZNSt8ios_base4InitC1Ev')) or not section or info & 15 != 2:
            continue
        p = next(p for p in ph if p['type'] == 1 and p['flags'] & 1
                 and p['vaddr'] <= value < p['vaddr'] + p['filesz'])
        code = span(elf, p['offset'] + value - p['vaddr'], size)
        if key == nid('_init_env'):
            evidence.update(symbol=symbol, address=hex(value), size=size, bytes=code.hex(),
                            init_env_is_ret=code == b'\xc3')
        else:
            evidence['ios_base_init'] = dict(symbol=symbol, address=hex(value), size=size,
                                             code_sha256=hashlib.sha256(code).hexdigest(),
                                             name='_ZNSt8ios_base4InitC1Ev')
    return evidence
