# SPDX-License-Identifier: GPL-2.0-or-later
"""Game and user save directory status and folder selection helpers."""

from pathlib import Path
from bbport_i18n import tr
from gi.repository import Gio, GLib, Gtk
from launcher.gtk.constants import DATA_DIR


def game_dir(win):
    return Path(win.settings["game_dir"]).expanduser()


def user_dir(win):
    u = win.settings["user_dir"]
    return Path(u).expanduser() if u else DATA_DIR / "user"


def update_user_status(win):
    path, custom = user_dir(win), bool(win.settings["user_dir"])
    win.user_row.set_subtitle(str(path) if custom else tr("Default: {}").format(path))
    saves = path / "savedata"
    found = saves.is_dir() and any(saves.iterdir())
    win.user_status.set_from_icon_name("object-select-symbolic" if found else "document-new-symbolic")
    win.user_status.set_tooltip_text(
        tr("Saves found") if found else tr("No saves yet: the game will create them here")
    )
    win.user_reset.set_sensitive(custom)


def choose_folder(win, title, current, done):
    dialog = Gtk.FileDialog(title=title)
    if Path(current).is_dir():
        dialog.set_initial_folder(Gio.File.new_for_path(str(current)))

    def finish(dialog, result):
        try:
            folder = dialog.select_folder_finish(result)
        except GLib.Error:
            return
        if folder:
            done(folder.get_path())
    dialog.select_folder(win, None, finish)


def on_choose_user(win, _button):
    def chosen(path):
        win.settings["user_dir"] = path
        update_user_status(win)
        win.store()
    choose_folder(win, tr("Saves folder"), user_dir(win), chosen)


def on_reset_user(win, _button):
    win.settings["user_dir"] = ""
    update_user_status(win)
    win.store()


def update_game_status(win):
    path = game_dir(win)
    ok = bool(win.settings["game_dir"]) and (path / "eboot.bin").is_file()
    win.game_row.set_subtitle(str(path) if win.settings["game_dir"] else tr("not chosen"))
    win.game_status.set_from_icon_name("object-select-symbolic" if ok else "dialog-warning-symbolic")
    win.game_status.set_tooltip_text(tr("eboot.bin found") if ok else tr("No eboot.bin in the folder"))
    win.launch_button.set_sensitive(ok or win.process is not None)


def on_choose_game(win, _button):
    def chosen(path):
        win.settings["game_dir"] = path
        update_game_status(win)
        win.store()
    choose_folder(win, tr("Game folder (with eboot.bin)"), game_dir(win), chosen)
