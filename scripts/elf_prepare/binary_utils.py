# SPDX-License-Identifier: GPL-2.0-or-later
"""Binary slicing, unpacking, and NID generation utilities."""

import base64
import hashlib
import struct

SALT = bytes.fromhex('518d64a635ded8c1e6b039b1c3e55230')


def span(data, offset, size):
    """Safely slice a contiguous span of bytes with bounds checking."""
    if offset < 0 or size < 0 or offset + size > len(data):
        raise ValueError(f"out of bounds: {offset:#x}+{size:#x} / {len(data):#x}")
    return data[offset:offset + size]


def unpack(fmt, data, offset):
    """Unpack a struct format from binary data at offset."""
    return struct.unpack(fmt, span(data, offset, struct.calcsize(fmt)))


def nid(name):
    """Calculate the Sony PS4 NID hash for an exported symbol name."""
    digest = hashlib.sha1(name.encode('ascii') + SALT).digest()[:8][::-1]
    return base64.b64encode(digest).decode().rstrip('=').replace('/', '-')
