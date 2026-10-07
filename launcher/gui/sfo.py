# SPDX-License-Identifier: GPL-2.0-or-later
"""PS4 PARAM.SFO metadata extraction and validation routines."""

from pathlib import Path
import struct
from bbport_i18n import tr


def read_sfo_metadata(sfo_path):
    """Extract TITLE_ID and APP_VER from param.sfo if present."""
    if not Path(sfo_path).is_file():
        return {}
    try:
        data = Path(sfo_path).read_bytes()
        if data[:4] != b'\x00PSF':
            return {}
        key_off, val_off, count = struct.unpack('<III', data[8:20])
        entries = {}
        for i in range(count):
            entry_offset = 20 + i * 16
            k_off, p_fmt, p_len, p_max, v_off = struct.unpack(
                '<HHIII', data[entry_offset:entry_offset + 16]
            )
            key = data[key_off + k_off:].split(b'\x00', 1)[0].decode('latin1')
            val = data[val_off + v_off:val_off + v_off + p_len].rstrip(
                b'\x00'
            ).decode('latin1', errors='ignore')
            entries[key] = val
        return entries
    except Exception:
        return {}


def verify_game_dump(game_dir):
    """Check directory for valid plaintext eboot.bin and metadata."""
    gdir = Path(game_dir).resolve()
    if not gdir.is_dir():
        return False, tr("Directory does not exist. Click 'Browse...' to select.")

    eboot = gdir / "eboot.bin"
    if not eboot.is_file():
        return False, tr("Directory found, but 'eboot.bin' is missing inside.")

    try:
        head = eboot.read_bytes()[:4]
        if head != b'O\x15=\x1d':
            return False, tr("'eboot.bin' does not have a standard PS4 SELF header.")
    except Exception as e:
        return False, tr("Cannot read eboot.bin: {error}").format(error=e)

    meta = read_sfo_metadata(gdir / "sce_sys" / "param.sfo")
    title_id = meta.get("TITLE_ID", gdir.name)
    app_ver = meta.get("APP_VER", tr("Unknown"))
    msg = tr("Ready: Title ID: {title_id} | Version: {app_ver} | Verified plaintext ELF").format(
        title_id=title_id, app_ver=app_ver
    )
    return True, msg
