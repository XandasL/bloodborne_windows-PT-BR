# SPDX-License-Identifier: GPL-2.0-or-later
"""XML patch encoder and instruction sequence compiler."""

import struct
import xml.etree.ElementTree as ET
from .constants import (
    EBOOT_BASE, OUTPUT_SIZE, RESOLUTION_TEMPLATE, SCENE_HEIGHT, SCENE_WIDTH,
    UI_HEIGHT, UI_WIDTH
)
from .segments import eboot_segments

__all__ = ["eboot_segments", "encode", "compile_patches", "resolution_writes"]


def encode(line):
    kind, value = line.get('Type'), line.get('Value')
    if kind == 'bytes':
        return bytes.fromhex(value.replace(' ', ''))
    if kind in ('bytes16', 'bytes32', 'bytes64'):
        return int(value, 0).to_bytes(int(kind[5:]) // 8, 'little')
    if kind == 'float32':
        return struct.pack('<f', float(value))
    if kind == 'float64':
        return struct.pack('<d', float(value))
    if kind == 'utf8':
        return value.encode() + b'\0'
    if kind == 'utf16':
        return value.encode('utf-16-le') + b'\0\0'
    raise ValueError(f'unsupported patch type {kind!r}')


def compile_patches(xml, names, app_version, segments):
    found = {}
    for meta in ET.parse(xml).getroot().iter('Metadata'):
        if meta.get('Name') in names and meta.get('AppVer') == app_version and meta.get('AppElf', 'eboot.bin') == 'eboot.bin':
            found[meta.get('Name')] = meta
    missing = [n for n in names if n not in found]
    if missing:
        raise ValueError(f'patches not found for app version {app_version}: {missing}')
    writes = []
    for name in names:
        for line in found[name].iter('Line'):
            offset = int(line.get('Address'), 0) - EBOOT_BASE
            data = encode(line)
            if not any(start <= offset and offset + len(data) <= end for start, end in segments):
                raise ValueError(f'{name}: address {line.get("Address")} is outside the eboot')
            writes.append((offset, data))
    return writes


def resolution_writes(xml, size, app_version, segments, ui=OUTPUT_SIZE):
    writes = compile_patches(xml, [RESOLUTION_TEMPLATE], app_version, segments)
    replacements = {
        SCENE_WIDTH: (0xB8, 0x500, size[0]),
        SCENE_HEIGHT: (0xB8, 0x2D0, size[1]),
        UI_WIDTH: (0xB8, 0x500, ui[0]),
        UI_HEIGHT: (0xB9, 0x2D0, ui[1])
    }
    out, seen = [], set()
    for offset, data in writes:
        if offset in replacements:
            opcode, old, value = replacements[offset]
            if data != bytes([opcode]) + old.to_bytes(3, 'little') or offset in seen:
                raise ValueError(f'unexpected resolution patch at {offset + EBOOT_BASE:#x}')
            data = bytes([opcode]) + value.to_bytes(3, 'little')
            seen.add(offset)
        out.append((offset, data))
    if seen != replacements.keys():
        raise ValueError('resolution patch is missing scene/UI viewport instructions')
    return out
