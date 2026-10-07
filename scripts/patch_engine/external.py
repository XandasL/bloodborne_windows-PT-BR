# SPDX-License-Identifier: GPL-2.0-or-later
"""Third-party shadPS4/GoldHEN external XML patch loader."""

import json
from pathlib import Path
import sys
import xml.etree.ElementTree as ET
from .constants import BLOODBORNE_IDS, EBOOT_BASE
from .xml_compiler import encode


DEFAULT_EXCLUDE = Path(__file__).resolve().parent.parent.parent / 'patches/Bloodborne.xml'


def external_patches(directory, app_version='01.09', exclude=DEFAULT_EXCLUDE):
    """Scan and parse third-party XML patches in directory."""
    found = []
    dir_p = Path(directory)
    exclude_p = Path(exclude).resolve() if exclude else None

    for path in sorted(dir_p.glob('*.xml')) if dir_p.is_dir() else []:
        if exclude_p and path.resolve() == exclude_p:
            continue
        try:
            root = ET.parse(path).getroot()
        except ET.ParseError as error:
            print(f'Patches: {path.name}: {error}', file=sys.stderr)
            continue
        ids = {e.text.strip() for e in root.iter('ID') if e.text}
        if ids and not ids & BLOODBORNE_IDS:
            continue
        for meta in root.iter('Metadata'):
            if meta.get('AppVer') == app_version and meta.get('AppElf', 'eboot.bin') == 'eboot.bin':
                found.append((f'{path.name}/{meta.get("Name")}', path, meta))
    return found


def external_selection(found, config):
    """Filter external patches against config enable/disable lists."""
    settings = {}
    if config and Path(config).is_file():
        settings = json.loads(Path(config).read_text())
    enabled, disabled = set(settings.get('enabled', [])), set(settings.get('disabled', []))
    return [(k, p, m) for k, p, m in found
            if k in enabled or (k not in disabled and m.get('isEnabled', 'false').lower() == 'true')]


def compile_external(selected, segments):
    """Compile selected third-party patches into byte write sequences."""
    writes = []
    for key, _, meta in selected:
        try:
            ours = []
            for line in meta.iter('Line'):
                offset = int(line.get('Address') or '', 0) - EBOOT_BASE
                data = encode(line)
                if not any(start <= offset and offset + len(data) <= end for start, end in segments):
                    raise ValueError(f'address {line.get("Address")} is outside the eboot')
                ours.append((offset, data))
        except ValueError as error:
            print(f'Patches: skipped {key}: {error}', file=sys.stderr)
            continue
        writes += ours
        print(f'Patches: external {key} ({len(ours)} writes)')
    return writes
