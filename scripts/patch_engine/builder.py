# SPDX-License-Identifier: GPL-2.0-or-later
"""Patch execution and output serialization coordinator."""

import struct
from .constants import EBOOT_BASE, OUTPUT_SIZE
from .external import compile_external, external_patches, external_selection
from .resolution import render_size
from .segments import eboot_segments
from .settings_helper import validate_patch_requirements
from .xml_compiler import compile_patches, resolution_writes


def execute_patch_compilation(a, names, settings):
    """Compile patches and serialize BBPATCH2 output binary."""
    validate_patch_requirements(names, a.game_dir)
    segments = eboot_segments((a.out / 'eboot.elf').read_bytes())
    writes = compile_patches(a.xml, names, a.app_version, segments)
    size = render_size(settings, a.render_res) if a.render_res else None
    if size:
        writes += resolution_writes(a.xml, size, a.app_version, segments, OUTPUT_SIZE)
        if size[0] * size[1] > OUTPUT_SIZE[0] * OUTPUT_SIZE[1]:
            writes += compile_patches(a.xml, ['Increased Graphics Heap Sizes'], a.app_version, segments)
    if a.patches_dir:
        ext = external_patches(a.patches_dir, a.app_version, a.xml)
        writes += compile_external(external_selection(ext, a.patches_config), segments)
    blob = struct.pack('<8sQQ', b'BBPATCH2', EBOOT_BASE, len(writes))
    for offset, data in writes:
        blob += struct.pack('<QQ', offset, len(data)) + data
    (a.out / 'patches.bin').write_bytes(blob)
    print(f'Patches: FPS preset {a.fps}; {len(writes)} writes from {names or "none"}')
