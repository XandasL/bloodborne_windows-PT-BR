# SPDX-License-Identifier: GPL-2.0-or-later
"""Boot binary unpacker for libc_link."""

from prepare import span, unpack


def setup_raw_boot(raw):
    """Unpack header and tables from boot.bin."""
    magic, size, entry, ns, nr, ni, flags = unpack('<8s6Q', raw, 0)
    if magic != b'BBPROBE2':
        raise ValueError('linker expects freshly prepared BBPROBE2')
    pos = 56
    segments = [unpack('<3Q', raw, pos + i * 24) for i in range(ns)]
    pos += ns * 24
    names = [span(raw, pos + i * 128, 128).split(b'\0', 1)[0].decode() for i in range(ni)]
    pos += ni * 128
    relocs = [unpack('<QQqq', raw, pos + i * 32) for i in range(nr)]
    pos += nr * 32
    image = bytearray(span(raw, pos, size))
    if pos + size != len(raw):
        raise ValueError('unexpected boot trailer')
    return (magic, size, entry, ns, nr, ni, flags), segments, names, relocs, image
