# SPDX-License-Identifier: GPL-2.0-or-later
"""Engine patch management and binary compiler package."""

from .constants import (
    BLOODBORNE_IDS, EBOOT_BASE, EFFECTS, FPS_PRESETS, MODEL_LOD, OUTPUT_SIZE,
    PRESET_SCALES, RESOLUTION_TEMPLATE, SCENE_HEIGHT, SCENE_WIDTH, UI_HEIGHT,
    UI_WIDTH
)
from .external import compile_external, external_patches, external_selection
from .resolution import output_size, render_size, scaled_sizes
from .segments import eboot_segments
from .settings_helper import effect_patches, read_settings, validate_patch_requirements
from .xml_compiler import compile_patches, encode, resolution_writes

__all__ = [
    "BLOODBORNE_IDS", "EBOOT_BASE", "EFFECTS", "FPS_PRESETS", "MODEL_LOD",
    "OUTPUT_SIZE", "PRESET_SCALES", "RESOLUTION_TEMPLATE", "SCENE_HEIGHT",
    "SCENE_WIDTH", "UI_HEIGHT", "UI_WIDTH", "compile_external", "compile_patches",
    "eboot_segments", "effect_patches", "encode", "external_patches",
    "external_selection", "output_size", "read_settings", "render_size",
    "resolution_writes", "scaled_sizes", "validate_patch_requirements"
]
