# SPDX-License-Identifier: GPL-2.0-or-later
"""Settings validation and reading for patches."""

from pathlib import Path
from .constants import EFFECTS, MODEL_LOD


def validate_patch_requirements(names, game):
    """Verify prerequisites and conflicting patch selections."""
    if 'Restore Debug Camera' in names and 'Enemy Control' in names:
        raise ValueError('Restore Debug Camera conflicts with Enemy Control; enable only one')
    if 'Restore Debug Menu (READ NOTES)' in names:
        font = Path(game) / 'dvdroot_ps4/font'
        missing = [name for name in ('DbgFont14h.ccm', 'DbgFont14h.tpf')
                   if not (font / name).is_file() or (font / name).stat().st_size == 0]
        if missing:
            raise ValueError(f'Debug menu needs non-empty font files in {font}: {", ".join(missing)}.')


def effect_patches(settings):
    """Retrieve enabled effect patch names from settings dictionary."""
    names = []
    for key, (off, on) in EFFECTS.items():
        if key not in settings:
            continue
        name = on if settings[key] == '1' else off
        if name:
            names.append(name)
    lod = MODEL_LOD.get(settings.get('model_lod', '0'))
    if lod:
        names.append(lod)
    return names


def read_settings(path):
    """Read key-value configuration from ini file."""
    settings = {}
    path = Path(path)
    if path.exists():
        for line in path.read_text().splitlines():
            key, sep, value = line.partition('=')
            if sep and not line.startswith('#'):
                settings[key.strip()] = value.strip()
    return settings
