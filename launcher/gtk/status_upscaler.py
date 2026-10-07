# SPDX-License-Identifier: GPL-2.0-or-later
"""Upscaler status and asset verification helper."""

import os
from pathlib import Path
from bbport_assets import fsr411_problem
from bbport_i18n import tr
from launcher.gtk.constants import DATA_DIR, PORT_DIR
from launcher.gtk.widgets import combo_value


def update_upscaler_status(win):
    value = combo_value(win.upscaler_row)
    if value == "fsr4":
        ok = (PORT_DIR / "fsr4_shaders").is_dir()
        hint = tr("Assets found") if ok else tr("No assets: tools/fetch_fsr4_assets.sh")
    elif value == "fsr411":
        env_dir = os.environ.get("BB_FSR411_DIR")
        directory = Path(env_dir) if env_dir else (
            PORT_DIR / "fsr4_411" if (PORT_DIR / "fsr4_411").is_dir() else (
                DATA_DIR / "fsr4_411"
            )
        )
        problem = fsr411_problem(
            directory, combo_value(win.output_row), int(combo_value(win.preset_row))
        )
        hint = tr("Assets for the selected mode found") if not problem else (
            tr("{}. Install the full fsr4_411 set into {}.").format(problem, directory)
        )
    else:
        hint = (
            tr("Anti-aliasing at the output resolution, no FSR model")
            if value == "taa" else None
        )
    win.preset_row.set_sensitive(value not in ("taa", "off"))
    win.sharpen_row.set_sensitive(value != "off")
    win.sharpness_row.set_sensitive(value != "off")
    win.upscaler_row.set_subtitle(hint or "")
