# SPDX-License-Identifier: GPL-2.0-or-later
"""Constants and tables for engine and graphics patch compilation."""

EBOOT_BASE = 0x400000

FPS_PRESETS = {
    '30': [],
    '60': ['60 FPS++'],
    '90': ['90 FPS++'],
    'uncap': ['Uncap FPS++']
}

OUTPUT_SIZE = (1920, 1080)
PRESET_SCALES = [1.0, 1.5, 1.7, 2.0, 3.0]
RESOLUTION_TEMPLATE = 'Resolution Patch 1280x720 (16:9)'

EFFECTS = {
    'effect_chromatic_aberration': ('Disable Chromatic Aberration', None),
    'effect_dof': ('Disable DoF', None),
    'effect_motion_blur': ('Disable Motion Blur (perf increase)', None),
    'effect_ssao': ('Disable SSAO', None),
    'effect_game_aa': ('Disable AA', None),
    'effect_dynamic_shadows': ('Disable Dynamic Light Shadows (perf increase)', None),
    'effect_ssr': (None, 'Enable Screen Space Reflections (READ NOTE)'),
    'skip_intro': (None, 'Skip Intro'),
    'debug_camera': (None, 'Restore Debug Camera'),
    'debug_menu': (None, 'Restore Debug Menu (READ NOTES)'),
}

MODEL_LOD = {
    '-2': 'Model LOD -2 (Highest)',
    '1': 'Model LOD 1 (Lower)',
    '2': 'Model LOD 2 (Lowest)'
}

SCENE_WIDTH = 0x02196A6B - EBOOT_BASE
SCENE_HEIGHT = 0x02196A7A - EBOOT_BASE
UI_WIDTH = 0x02358554 - EBOOT_BASE
UI_HEIGHT = 0x0235855D - EBOOT_BASE

BLOODBORNE_IDS = {'CUSA00207', 'CUSA00208', 'CUSA00900', 'CUSA01363', 'CUSA03173', 'CUSA03023'}
