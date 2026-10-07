# SPDX-License-Identifier: GPL-2.0-or-later
"""Third-party patch profile management and discovery."""

import json
from bbport_i18n import tr
from gi.repository import Adw, GLib
from launcher.gtk.config import patches_dir
from launcher.gtk.constants import DATA_DIR
from launcher.gtk.status_paths import choose_folder
try:
    from scripts.patch_engine.external import external_patches
except ImportError:
    from patches import external_patches


def save_patch_profile(win):
    """Save patches.json switches that differ from each file's isEnabled."""
    enabled, disabled = [], []
    for key, row in win.patch_list.rows:
        if row.get_active() != row.default:
            (enabled if row.get_active() else disabled).append(key)
    path = DATA_DIR / "patches.json"
    try:
        profile = json.loads(path.read_text())
    except (OSError, ValueError):
        profile = {}
    shown = {key for key, _ in win.patch_list.rows}
    profile = {
        "enabled": sorted({k for k in profile.get("enabled", []) if k not in shown} | set(enabled)),
        "disabled": sorted({k for k in profile.get("disabled", []) if k not in shown} | set(disabled)),
    }
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(profile, indent=2, ensure_ascii=False) + "\n")


def refresh_patches(win):
    win.patch_list.clear()
    directory = patches_dir(win.settings)
    try:
        profile = json.loads((DATA_DIR / "patches.json").read_text())
    except (OSError, ValueError):
        profile = {}
    found = external_patches(directory)
    for key, _path, meta in found:
        default = meta.get("isEnabled", "false").lower() == "true"
        active = key in profile.get("enabled", []) or (default and key not in profile.get("disabled", []))
        subtitle = key.split("/", 1)[0]
        if meta.get("Author"):
            subtitle += " · " + tr("Author: {}").format(meta.get("Author"))
        row = Adw.SwitchRow(
            title=GLib.markup_escape_text(meta.get("Name") or key),
            subtitle=GLib.markup_escape_text(subtitle), active=active
        )
        if meta.get("Note"):
            row.set_tooltip_text(meta.get("Note").replace("\\n", "\n"))
        row.default = default
        win.patch_list.add(key, row)
    win.patches_folder_row.set_subtitle(
        str(directory) if found else f"{directory} — {tr('No patches')}"
    )


def on_refresh_patches(win, _button):
    save_patch_profile(win)
    refresh_patches(win)


def on_choose_patches(win, _button):
    save_patch_profile(win)

    def chosen(path):
        win.settings["patches_dir"] = path
        refresh_patches(win)
        win.store()
    choose_folder(win, tr("Patches folder"), patches_dir(win.settings), chosen)
