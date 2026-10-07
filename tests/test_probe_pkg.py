# SPDX-License-Identifier: GPL-2.0-or-later
"""Packaging utilities for loader synthetic tests."""

import struct


def package(code, relocs=(), names=(), capabilities=None):
    image = code.ljust(4096, b'\0')
    magic = b'BBPROBE1' if capabilities is None else b'BBPROBE2'
    header = struct.pack('<8sQQQQQ', magic, len(image), 0, 1, len(relocs), len(names))
    if capabilities is not None:
        header += struct.pack('<Q', capabilities)
    segment = struct.pack('<QQQ', 0, len(image), 5)
    return (header + segment + b''.join(n.encode().ljust(128, b'\0') for n in names)
            + b''.join(struct.pack('<QQqq', *r) for r in relocs) + image)


def native_package(name='fixture-native', binding_address=4112, binding_kind=1,
                   lib_flags=5, init=b'\x31\xc0\xc3', native=b'\xc3', metadata=None):
    code = b'\x48\x83\xec\x08\xff\x15\x16\0\0\0\x48\x83\xc4\x08\xff\x25\x14\0\0\0'
    image = code.ljust(4096, b'\0') + init.ljust(16, b'\0') + native
    image = image.ljust(12288, b'\0')
    header = struct.pack('<8s6Q', b'BBPROBE3', len(image), 0, 3, 2, 2, 1)
    meta = metadata or (4096, 8192, 4096, 8192, 32, 4, 1, 256)
    return (header + struct.pack('<8Q', *meta) + struct.pack('<3Q', 0, binding_address, binding_kind)
            + struct.pack('<9Q', 0, 4096, 5, 4096, 4096, lib_flags, 8192, 4096, 6)
            + name.encode().ljust(128, b'\0') + b'after-native'.ljust(128, b'\0')
            + struct.pack('<8Q', 32, 1, 0, 0, 40, 1, 1, 0) + image)
