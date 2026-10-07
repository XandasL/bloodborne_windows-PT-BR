# SPDX-License-Identifier: GPL-2.0-or-later
"""Custom GTK / Adwaita helper widgets and utilities."""

from pathlib import Path
from bbport_i18n import tr
from gi.repository import Adw, Gio, Gtk


def flat_button(icon, tooltip, handler):
    button = Gtk.Button(icon_name=icon, valign=Gtk.Align.CENTER, tooltip_text=tooltip)
    button.add_css_class("flat")
    button.connect("clicked", handler)
    return button


def open_folder(window, path):
    """Opens path in the system file manager, creating it if necessary."""
    try:
        Path(path).mkdir(parents=True, exist_ok=True)
    except OSError:
        return
    Gtk.FileLauncher.new(Gio.File.new_for_path(str(path))).launch(window, None, None)


def combo_row(title, subtitle, choices, current):
    model = Gtk.StringList.new([tr(label) for label, _ in choices])
    row = Adw.ComboRow(title=title, model=model)
    if subtitle:
        row.set_subtitle(subtitle)
    values = [value for _, value in choices]
    row.set_selected(values.index(current) if current in values else 0)
    row.values = values
    return row


def combo_value(row):
    return row.values[row.get_selected()]


class FolderList:
    """Preferences group with folder action row and switch rows below it."""

    def __init__(self, group):
        self.group = group
        self.rows = []

    def clear(self):
        for _, row in self.rows:
            self.group.remove(row)
        self.rows = []

    def add(self, key, row):
        self.group.add(row)
        self.rows.append((key, row))
