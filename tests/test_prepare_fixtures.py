# SPDX-License-Identifier: GPL-2.0-or-later
"""Test fixtures for prepare tests."""

import struct


def make_self_fixture():
    data = bytearray(0x210)
    data[:4] = b'O\x15=\x1d'
    struct.pack_into('<H', data, 24, 1)
    struct.pack_into('<QQQQ', data, 32, 0x800, 0x200, 16, 16)
    struct.pack_into('<16sHHIQQQIHHHHHH', data, 64,
                     b'\x7fELF\x02\x01\x01\x09', 0xfe10, 62, 1, 0, 64, 0, 0, 64, 56, 1, 0, 0, 0)
    struct.pack_into('<IIQQQQQQ', data, 128, 1, 5, 0x1000, 0, 0, 16, 32, 0x1000)
    data[0x200:] = bytes(range(16))
    return data


def make_libc_fixture(instruction):
    data = bytearray(0x320)
    data[:4] = b'O\x15=\x1d'
    struct.pack_into('<H', data, 24, 2)
    struct.pack_into('<QQQQ', data, 32, 0x800, 0x200, 16, 16)
    struct.pack_into('<QQQQ', data, 64, 0x100800, 0x220, 256, 256)
    struct.pack_into('<16sHHIQQQIHHHHHH', data, 96, b'\x7fELF\x02\x01\x01\x09', 0xfe18, 62, 1, 0, 64, 0, 0, 64, 56, 3, 0, 0, 0)
    struct.pack_into('<IIQQQQQQ', data, 160, 1, 5, 0x1000, 0, 0, 16, 16, 0x1000)
    struct.pack_into('<IIQQQQQQ', data, 216, 0x61000000, 4, 0x2000, 0, 0, 256, 0, 16)
    struct.pack_into('<IIQQQQQQ', data, 272, 2, 4, 0x20b0, 0, 0, 80, 80, 8)
    data[0x200] = instruction
    name = b'bzQExy189ZI#C#A\0'
    data[0x220:0x220 + len(name)] = name
    struct.pack_into('<IBBHQQ', data, 0x260, 0, 0x12, 0, 1, 0, 1)
    for i, (tag, val) in enumerate([(0x61000035, 0), (0x61000037, 32), (0x61000039, 64), (0x6100003f, 24), (0, 0)]):
        struct.pack_into('<QQ', data, 0x2d0 + 16 * i, tag, val)
    return data
