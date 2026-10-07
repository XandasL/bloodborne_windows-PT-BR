# SPDX-License-Identifier: GPL-2.0-or-later
"""Bloodborne GTK4 / Libadwaita main application window."""

from bbport_i18n import set_language
from gi.repository import Adw
from launcher.gtk.config import load_ini, load_settings
from launcher.gtk.window_builder import build_main_window, update_launch_button
from launcher.gtk.window_mods_mixin import WindowModsMixin
from launcher.gtk.window_process_mixin import WindowProcessMixin
from launcher.gtk.window_status_mixin import WindowStatusMixin


class LauncherWindow(
    Adw.ApplicationWindow, WindowModsMixin, WindowProcessMixin, WindowStatusMixin
):
    """Main application window uniting all GUI behaviors and subviews."""

    def __init__(self, app):
        super().__init__(application=app, title="Bloodborne")
        self.set_default_size(760, 820)
        self.settings = load_settings()
        set_language(self.settings.get("ui_language", ""))
        self.ini, self.ini_lines = load_ini()
        self.process = None
        self.stream = None
        self.build()
        self.connect("close-request", self.on_close)

    def build(self):
        """Recreates the window's content in the active launcher language."""
        build_main_window(self)

    def update_launch_button(self):
        """Updates the label and style of the primary launch action button."""
        update_launch_button(self)
