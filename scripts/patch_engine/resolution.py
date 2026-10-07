# SPDX-License-Identifier: GPL-2.0-or-later
"""Resolution calculations for upscalers and display outputs."""

from .constants import OUTPUT_SIZE, PRESET_SCALES


def render_size(settings, override=''):
    """Calculate internal render resolution according to upscaler preset."""
    if override:
        w, h = (int(v) for v in override.lower().split('x'))
        return (w, h)
    if settings.get('upscaler', 'fsr3') == 'off':
        return None
    preset = int(settings.get('preset', '0') or 0)
    scale = PRESET_SCALES[max(0, min(preset, len(PRESET_SCALES) - 1))]
    if scale == 1.0:
        return None
    return tuple(max(2, round(v / scale / 2) * 2) for v in OUTPUT_SIZE)


def output_size(settings):
    """Output (UI) size from bbport.ini output_res, e.g. 3840x2160."""
    try:
        w, h = (int(v) for v in settings.get('output_res', '').lower().split('x'))
        if w > 0 and h > 0:
            return (w, h)
    except ValueError:
        pass
    return OUTPUT_SIZE


def scaled_sizes(settings):
    """Compute (render, output) resolution for display outputs other than 1080p."""
    out = output_size(settings)
    if out == OUTPUT_SIZE or settings.get('upscaler') == 'taa':
        return None
    scale = 1.0
    if settings.get('upscaler', 'fsr3') != 'off':
        preset = int(settings.get('preset', '0') or 0)
        scale = PRESET_SCALES[max(0, min(preset, len(PRESET_SCALES) - 1))]
    render = tuple(max(2, round(v / scale / 2) * 2) for v in out)
    if render == OUTPUT_SIZE:
        render = (1916, 1078)
    return render, out
