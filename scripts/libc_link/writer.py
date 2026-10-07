# SPDX-License-Identifier: GPL-2.0-or-later
"""Binary packing and linking report generator for libc_link."""

import collections
import json
import struct


def write_boot_libc(out, header_info, metadata, main_tls_values, bindings,
                     segments, names, relocs, image):
    """Write output binary file."""
    with (out / 'boot-libc.bin').open('wb') as f:
        f.write(struct.pack('<8s6Q', *header_info))
        f.write(struct.pack('<8Q', *metadata))
        f.write(struct.pack('<4Q', *main_tls_values))
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


def write_report(out, libc, base, libsize, metadata, bindings, tls_count,
                 patched, main_tls_values, tls, names):
    """Generate and write libc-link.json report."""
    report = dict(
        base=hex(base), size=libsize, sha256=libc['sha256'],
        init=hex(metadata[2]), bindings=len(bindings), tls_module_id=2,
        fs_loads_patched=patched,
        main_tls=dict(zip(('vaddr', 'filesz', 'memsz', 'align'), main_tls_values)),
        tls_relocations=tls_count,
        tls_template_bytes=tls['filesz'], tls_memory_bytes=tls['memsz'],
        imports=names,
        relocation_counts=dict(collections.Counter(r[1] for r in libc['relocs'])),
        symbol_bindings=[dict(import_name=names[i], address=hex(a), kind=k)
                         for i, a, k in bindings]
    )
    (out / 'libc-link.json').write_text(json.dumps(report, indent=2) + '\n')
