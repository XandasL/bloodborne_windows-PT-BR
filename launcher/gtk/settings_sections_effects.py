# SPDX-License-Identifier: GPL-2.0-or-later
"""Game effects and frame rate settings sections."""

from bbport_i18n import tr
from gi.repository import Adw
from launcher.gtk.constants import EFFECTS, FPS_MODES, MODEL_LOD
from launcher.gtk.widgets import combo_row


def build_effects_group(win):
    effects = Adw.PreferencesGroup(
        title=tr("Game effects"), description=tr("Game patches, applied at start")
    )
    win.lod_row = combo_row(
        tr("Model detail"), None, MODEL_LOD, win.ini.get("model_lod", "0")
    )
    effects.add(win.lod_row)
    win.effect_rows = {}
    for key, title, default in EFFECTS:
        row = Adw.SwitchRow(
            title=tr(title),
            active=win.ini.get(key, "1" if default else "0") == "1"
        )
        if key == "debug_menu":
            row.set_subtitle(tr(
                "Install DbgFont14h.ccm and DbgFont14h.tpf into dvdroot_ps4/font "
                "from Nexus mod #253"
            ))
        win.effect_rows[key] = row
        effects.add(row)
    return effects


def build_frames_group(win):
    frames = Adw.PreferencesGroup(title=tr("Frame rate"))
    win.fps_row = combo_row(
        tr("Mode"), tr("Which frame rate patch to apply to the game"),
        FPS_MODES, win.settings["fps_mode"]
    )
    frames.add(win.fps_row)
    win.limit_row = Adw.SpinRow.new_with_range(0, 480, 1)
    win.limit_row.set_title(tr("FPS limit"))
    win.limit_row.set_subtitle(tr(
        "0: the display refresh rate (up to 120 Hz); set a number for another limit"
    ))
    win.limit_row.set_value(win.settings["fps_limit"])
    frames.add(win.limit_row)
    return frames
