# SPDX-License-Identifier: GPL-2.0-or-later
"""Test fixtures and helpers for patch compilation tests."""

import struct
from paths import ROOT

XML = ROOT / 'patches/Bloodborne.xml'
SEGMENTS = [(0, 0x6000000)]


def immediate(writes, address):
    """Extract opcode and immediate argument from instruction writes."""
    data = bytearray(9)
    for offset, value in writes:
        if address <= offset < address + 9:
            data[offset - address:offset - address + len(value)] = value
    assert data[5:] == b'\x0f\x1f\x40\x00'
    return data[0], struct.unpack_from('<I', data, 1)[0]


EXTERNAL_XML = """<?xml version="1.0"?>
<Patch>
  <TitleID><ID>CUSA03173</ID><ID>CUSA00207</ID></TitleID>
  <Metadata Title="Bloodborne" Name="On" Author="x" PatchVer="1.0" AppVer="01.09" AppElf="eboot.bin" isEnabled="true">
    <PatchList><Line Type="bytes" Address="0x00401000" Value="9090"/></PatchList>
  </Metadata>
  <Metadata Title="Bloodborne" Name="Off" Author="x" PatchVer="1.0" AppVer="01.09" AppElf="eboot.bin">
    <PatchList><Line Type="bytes32" Address="0x00402000" Value="0x12345678"/></PatchList>
  </Metadata>
  <Metadata Title="Bloodborne" Name="Mask" Author="x" PatchVer="1.0" AppVer="01.09" AppElf="eboot.bin" isEnabled="true">
    <PatchList><Line Type="mask" Value="90 ?? 90" Offset="0"/></PatchList>
  </Metadata>
  <Metadata Title="Bloodborne" Name="Old" Author="x" PatchVer="1.0" AppVer="01.00" AppElf="eboot.bin" isEnabled="true">
    <PatchList><Line Type="bytes" Address="0x00403000" Value="90"/></PatchList>
  </Metadata>
</Patch>"""
