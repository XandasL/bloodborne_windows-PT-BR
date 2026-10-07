# SPDX-License-Identifier: GPL-2.0-or-later
"""Binary serialization and metadata report for guest modules linker."""

import json
from pathlib import Path
import struct


def write_boot_linked(out, image, entry, segments, relocs, names, flags,
                      procparam, main_tls_values, table, bindings):
    """Serialize BBPROBE5 multi-module container."""
    with (out / 'boot-linked.bin').open('wb') as f:
        f.write(struct.pack('<8s6Q', b'BBPROBE5', len(image), entry,
                            len(segments), len(relocs), len(names), flags))
        f.write(struct.pack('<Q', procparam['vaddr']))
        f.write(struct.pack('<4Q', *main_tls_values))
        f.write(struct.pack('<Q', len(table)))
        for t in table:
            f.write(struct.pack('<7Q', t['base'], t['size'], t['init'],
                                t['tls_address'], t['tls_memsz'],
                                t['tls_filesz'], t['tls_module']))
        f.write(struct.pack('<Q', len(bindings)))
        for binding in bindings:
            f.write(struct.pack('<3Q', *binding))
        for segment in segments:
            f.write(struct.pack('<3Q', *segment))
        for name in names:
            if len(name.encode()) >= 128:
                raise ValueError('name too long')
            f.write(name.encode().ljust(128, b'\0'))
        for relocation in relocs:
            f.write(struct.pack('<QQqq', *relocation))
        f.write(image)


def write_report(out, table, bindings, names, fs_patched, main_tls_values, unresolved):
    """Write link.json summary report."""
    report = dict(
        modules=[{k: (hex(v) if k in ('base', 'init', 'tls_address') else v)
                  for k, v in t.items()} for t in table],
        bindings=len(bindings), imports=len(names), fs_loads_patched=fs_patched,
        main_tls=dict(zip(('vaddr', 'filesz', 'memsz', 'align'), main_tls_values)),
        unresolved_imports=unresolved
    )
    (out / 'link.json').write_text(json.dumps(report, indent=2) + '\n')
