# SPDX-License-Identifier: GPL-2.0-or-later
"""Basic settings sections: launcher language, game and user folders."""

from bbport_i18n import tr
from gi.repository import Adw, Gtk
from launcher.gtk.constants import LANGUAGES, UI_LANGUAGES
from launcher.gtk.widgets import combo_row, flat_button, open_folder


def build_launcher_group(win):
    group = Adw.PreferencesGroup()
    win.ui_language_row = combo_row(
        tr("Launcher language") + " / Launcher language",
        None, UI_LANGUAGES, win.settings.get("ui_language", "")
    )
    win.ui_language_row.connect("notify::selected", win.on_ui_language)
    group.add(win.ui_language_row)
    return group


def build_game_group(win):
    game = Adw.PreferencesGroup(title=tr("Game"))
    win.game_row = Adw.ActionRow(title=tr("Game folder (CUSA03173)"))
    win.game_status = Gtk.Image()
    win.game_row.add_suffix(win.game_status)
    win.game_row.add_suffix(flat_button(
        "folder-open-symbolic", tr("Choose the folder with eboot.bin"),
        win.on_choose_game
    ))
    win.game_row.add_suffix(flat_button(
        "system-file-manager-symbolic", tr("Open in the file manager"),
        lambda _b: open_folder(win, win.game_dir())
    ))
    game.add(win.game_row)

    win.user_row = Adw.ActionRow(title=tr("Saves folder"))
    win.user_status = Gtk.Image()
    win.user_row.add_suffix(win.user_status)
    win.user_row.add_suffix(flat_button(
        "folder-open-symbolic", tr("Choose the saves folder"), win.on_choose_user
    ))
    win.user_row.add_suffix(flat_button(
        "system-file-manager-symbolic", tr("Open in the file manager"),
        lambda _b: open_folder(win, win.user_dir())
    ))
    win.user_reset = flat_button(
        "edit-undo-symbolic", tr("Back to the default folder"), win.on_reset_user
    )
    win.user_row.add_suffix(win.user_reset)
    game.add(win.user_row)

    win.language_row = combo_row(
        tr("System language"), None, LANGUAGES, win.settings["language"]
    )
    game.add(win.language_row)
    return game
