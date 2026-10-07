# SPDX-License-Identifier: GPL-2.0-or-later
"""Test fixture and helper runner for link_libc tests."""

import json
from pathlib import Path
import tempfile
from unittest.mock import patch
import link_libc
from test_probe import package


def make_fixture(version=1, relocs=()):
    main_key = ('fixture', ('libc', 1), ('libc', 1))
    lib_key = ('fixture', ('libc', version), ('libc', 1))
    main = dict(identity=lambda name: main_key, libraries={'q': ('libc', 1)},
                modules={'q': ('libc', 1)}, ph=[dict(type=0x61000001, vaddr=256)])
    symbols = [dict(name='', type=3, binding=0, section=1, value=0, size=0, identity=None),
               dict(name='fixture#C#A', type=2, binding=1, section=1, value=16, size=1, identity=lib_key)]
    elf = b'\x31\xc0\xc3' + bytes(13) + b'\xc3' + bytes(111)
    libc = dict(elf=elf, ph=[dict(type=1, vaddr=0, memsz=4096, filesz=len(elf), offset=0, flags=5),
                             dict(type=7, vaddr=128, memsz=32, filesz=4)],
                tags={12: 0}, symbols=symbols, relocs=list(relocs), sha256='fixture')
    return main, libc


def run_link(main, libc, code=b'\xc3'):
    with tempfile.TemporaryDirectory() as tmp:
        out = Path(tmp)
        (out / 'boot.bin').write_bytes(package(code, names=['fixture#q#q'], capabilities=1))
        with patch.object(link_libc, 'module', side_effect=[main, libc]):
            link_libc.link(Path('fixture-game'), out)
        return (out / 'boot-libc.bin').read_bytes(), json.loads((out / 'libc-link.json').read_text())
