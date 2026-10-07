# SPDX-License-Identifier: GPL-2.0-or-later
"""State persistence and settings serialization for GTK launcher."""

from bbport_i18n import set_language, tr
from gi.repository import Adw, GLib
from launcher.gtk.config import load_ini, save_ini, save_settings
from launcher.gtk.widgets import combo_value


def store(win):
    s = win.settings
    s["language"] = combo_value(win.language_row)
    s["fullscreen"] = win.fullscreen_row.get_active()
    s["present_mode"] = combo_value(win.present_row)
    s["hdr"] = win.hdr_row.get_active()
    s["fps_mode"] = combo_value(win.fps_row)
    s["fps_limit"] = int(win.limit_row.get_value())
    s["draw_pipe"] = combo_value(win.pipe_row)
    s["readbacks"] = combo_value(win.readbacks_row)
    s["mangohud"] = win.mangohud_row.get_active()
    s["frame_stats"] = win.stats_row.get_active()
    s["gpu_profile"] = win.profile_row.get_active()
    s["vk_validation"] = win.validation_row.get_active()
    s["extra_env"] = win.extra_row.get_text().strip()
    s["mods_enabled"] = win.mods_enabled_row.get_active()
    win.save_mod_profile()
    win.save_patch_profile()
    save_settings(s)
    win.ini.update({
        "upscaler": combo_value(win.upscaler_row),
        "preset": str(combo_value(win.preset_row)),
        "sharpen": "1" if win.sharpen_row.get_active() else "0",
        "sharpness": f"{win.sharpness_row.get_value():.2f}",
        "object_motion": "1" if win.motion_row.get_active() else "0",
        "show_fps": "1" if win.show_fps_row.get_active() else "0",
        "output_res": combo_value(win.output_row),
        "model_lod": combo_value(win.lod_row),
        "live_resolution": combo_value(win.live_row),
        **{key: "1" if row.get_active() else "0" for key, row in win.effect_rows.items()},
    })
    save_ini(win.ini, win.ini_lines)
    win.ini, win.ini_lines = load_ini()


def on_ui_language(win, row, _param):
    choice = combo_value(row)
    if choice == win.settings.get("ui_language", ""):
        return
    win.store()
    win.settings["ui_language"] = choice
    save_settings(win.settings)
    if win.process:
        set_language(choice)
        win.toasts.add_toast(Adw.Toast(
            title=tr("Applies after restarting the launcher while the game is running")
        ))
        return
    set_language(choice)
    GLib.idle_add(lambda: win.build() and False)
