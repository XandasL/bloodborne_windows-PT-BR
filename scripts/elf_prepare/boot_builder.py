# SPDX-License-Identifier: GPL-2.0-or-later
"""Bloodborne eboot preparation and packaging coordinator."""

from pathlib import Path
from .binary_utils import nid, span, unpack
from .libc_inspector import inspect_libc
from .relocs import process_relocs, process_symbols
from .self_parser import parse_self
from .writer import KNOWN_CANDIDATES, write_analysis, write_boot_bin


def prepare(game, out):
    """Inspect and unpack game eboot into boot binary and debug image."""
    source = (game / 'eboot.bin').read_bytes()
    elf, header, ph, segments, missing = parse_self(source)
    loads = [p for p in ph if p['type'] in (1, 0x61000010)]

    def mapped(addr, size=8):
        return any(p['vaddr'] <= addr and addr + size <= p['vaddr'] + p['memsz'] for p in loads)

    size = max(p['vaddr'] + p['memsz'] for p in loads)
    if size > 512 * 1024 * 1024 or not mapped(header[4], 1):
        raise ValueError('invalid memory image')
    image = bytearray(size)
    for p in loads:
        if p['filesz'] > p['memsz']:
            raise ValueError('segment file size exceeds memory size')
        image[p['vaddr']:p['vaddr'] + p['filesz']] = span(elf, p['offset'], p['filesz'])
    dp = next(p for p in ph if p['type'] == 2)
    dyn = []
    for pos in range(dp['offset'], dp['offset'] + dp['filesz'], 16):
        tag, value = unpack('<QQ', elf, pos)
        if tag == 0:
            break
        dyn.append((tag, value))
    tags = dict(dyn)
    lib = next(p for p in ph if p['type'] == 0x61000000)
    blob = span(elf, lib['offset'], lib['filesz'])
    strings = span(blob, tags[0x61000035], tags[0x61000037])

    def string(off):
        span(strings, off, 1)
        return strings[off:strings.index(0, off)].decode('ascii')

    symbols = process_symbols(blob, tags)
    relocs, counts, imports = process_relocs(blob, tags, symbols, mapped, image)
    names = [symbols[i]['name'] for i in imports]
    known = {nid(name): name for name in KNOWN_CANDIDATES}
    libc_ev = inspect_libc(game / 'sce_module/libc.prx')
    out.mkdir(parents=True, exist_ok=True)
    write_boot_bin(out, size, header[4], loads, relocs, names,
                   libc_ev['init_env_is_ret'], image)
    (out / 'eboot.elf').write_bytes(elf)
    (out / 'entry.bin').write_bytes(image[:1024])
    rep = write_analysis(out, game, source, header, size, ph, segments, missing,
                         dyn, string, counts, names, known, libc_ev)
    print(f"{rep['sfo'].get('TITLE')} | entry={header[4]:#x} | image={size:,} bytes")
    print(f"{len(names)} imported symbols; {sum(counts.values()):,} relocations; {len(rep['needed'])} required modules")
    print(f"Unavailable non-loadable metadata headers: {missing}; not a byte-exact ELF reconstruction")
    print(f"Output: {out.resolve()}")
    print(f"libc _init_env verified RET: {libc_ev['init_env_is_ret']}")
