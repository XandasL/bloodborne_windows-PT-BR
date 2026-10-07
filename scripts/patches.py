#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Compile selected shadPS4/GoldHEN XML patches into out/patches.bin for the loader."""

import argparse
import os
from pathlib import Path
import sys

_scripts_dir = Path(__file__).resolve().parent
if str(_scripts_dir) not in sys.path:
    sys.path.insert(0, str(_scripts_dir))

from patch_engine import (  # noqa: E402
    BLOODBORNE_IDS, EBOOT_BASE, EFFECTS, FPS_PRESETS, MODEL_LOD, OUTPUT_SIZE,
    PRESET_SCALES, RESOLUTION_TEMPLATE, SCENE_HEIGHT, SCENE_WIDTH, UI_HEIGHT,
    UI_WIDTH, compile_external, compile_patches, eboot_segments, effect_patches,
    encode, external_patches, external_selection, output_size, read_settings,
    render_size, resolution_writes, scaled_sizes, validate_patch_requirements
)
from patch_engine.builder import execute_patch_compilation  # noqa: E402

__all__ = [
    "BLOODBORNE_IDS", "EBOOT_BASE", "EFFECTS", "FPS_PRESETS", "MODEL_LOD",
    "OUTPUT_SIZE", "PRESET_SCALES", "RESOLUTION_TEMPLATE", "SCENE_HEIGHT",
    "SCENE_WIDTH", "UI_HEIGHT", "UI_WIDTH", "compile_external", "compile_patches",
    "eboot_segments", "effect_patches", "encode", "external_patches",
    "external_selection", "output_size", "read_settings", "render_size",
    "resolution_writes", "scaled_sizes", "validate_patch_requirements"
]


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--patches-dir', type=Path)
    p.add_argument('--patches-config', type=Path)
    p.add_argument('--xml', type=Path, default=Path(__file__).resolve().parent.parent / 'patches/Bloodborne.xml')
    p.add_argument('--fps', choices=sorted(FPS_PRESETS), default='uncap')
    p.add_argument('--extra', default='')
    p.add_argument('--app-version', default='01.09')
    p.add_argument('--out', type=Path, default=Path(__file__).resolve().parent.parent / 'out')
    p.add_argument('--settings', type=Path, default=Path(__file__).resolve().parent.parent / 'bbport.ini')
    p.add_argument('--game-dir', type=Path, default=Path(os.environ.get('BB_GAME_DIR', '../CUSA03173')))
    p.add_argument('--render-res', default='')
    p.add_argument('--print-preset-size', action='store_true')
    p.add_argument('--output-res', default='')
    p.add_argument('--print-scaled', action='store_true')
    a = p.parse_args()
    st = read_settings(a.settings)
    if a.print_scaled:
        sz = scaled_sizes(st)
        if sz: print(f'{sz[0][0]}x{sz[0][1]} {sz[1][0]}x{sz[1][1]}')
        return
    if a.print_preset_size:
        sz = render_size(st)
        if sz: print(f'{sz[0]}x{sz[1]}')
        return
    names = FPS_PRESETS[a.fps] + [n.strip() for n in a.extra.split(';') if n.strip()]
    names += [n for n in effect_patches(st) if n not in names]
    execute_patch_compilation(a, names, st)


if __name__ == '__main__':
    main()
