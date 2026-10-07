# SPDX-License-Identifier: GPL-2.0-or-later
"""Eboot program headers and loadable segment extractor."""

import struct


def eboot_segments(elf):
    """Return loadable virtual address ranges [(start, end)] from ELF headers."""
    phoff, = struct.unpack_from('<Q', elf, 0x20)
    phentsize, phnum = struct.unpack_from('<HH', elf, 0x36)
    segments = []
    for i in range(phnum):
        kind, _, _, vaddr, _, _, memsz, _ = struct.unpack_from(
            '<IIQQQQQQ', elf, phoff + i * phentsize
        )
        if kind == 1:
            segments.append((vaddr, vaddr + memsz))
    return segments
