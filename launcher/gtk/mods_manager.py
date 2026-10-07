# SPDX-License-Identifier: GPL-2.0-or-later
"""Mods directory, profile management, and ordering."""

import json
from pathlib import Path
from bbport_i18n import tr
from gi.repository import Adw
from launcher.gtk.constants import DATA_DIR
from launcher.gtk.status_paths import choose_folder
from launcher.gtk.widgets import flat_button
try:
    from scripts.mod_manager.discovery import discover as discover_mods
except ImportError:
    from mods import discover as discover_mods


def mods_dir(win):
    return Path(win.settings.get("mods_dir") or DATA_DIR / "mods").expanduser()


def save_mod_profile(win):
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    rows = win.mod_list.rows
    profile = {
        "order": [n for n, _ in rows],
        "disabled": [n for n, r in rows if not r.get_active()],
    }
    (DATA_DIR / "mods.json").write_text(json.dumps(profile, indent=2, ensure_ascii=False) + "\n")


def refresh_mods(win):
    win.mod_list.clear()
    win.mods_folder_row.set_subtitle(str(mods_dir(win)))
    try:
        profile = json.loads((DATA_DIR / "mods.json").read_text())
    except (OSError, ValueError):
        profile = {}
    available = discover_mods(mods_dir(win))
    order = list(dict.fromkeys(n for n in [*profile.get("order", []), *available] if n in available))
    for name in order:
        row = Adw.SwitchRow(title=name, active=name not in profile.get("disabled", []))
        row.add_suffix(flat_button("go-up-symbolic", tr("Load earlier"), lambda _b, n=name: move_mod(win, n, -1)))
        row.add_suffix(flat_button("go-down-symbolic", tr("Load later"), lambda _b, n=name: move_mod(win, n, 1)))
        win.mod_list.add(name, row)
    if not order:
        win.mods_folder_row.set_subtitle(f"{mods_dir(win)} — {tr('No mods')}")


def move_mod(win, name, direction):
    rows = win.mod_list.rows
    idx = next(i for i, (n, _) in enumerate(rows) if n == name)
    tgt = idx + direction
    if 0 <= tgt < len(rows):
        rows[idx], rows[tgt] = rows[tgt], rows[idx]
        save_mod_profile(win)
        refresh_mods(win)


def on_refresh_mods(win, _button):
    save_mod_profile(win)
    refresh_mods(win)


def on_choose_mods(win, _button):
    save_mod_profile(win)

    def chosen(path):
        win.settings["mods_dir"] = path
        refresh_mods(win)
        win.store()
    choose_folder(win, tr("Mods folder"), mods_dir(win), chosen)
