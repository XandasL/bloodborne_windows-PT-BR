# SPDX-License-Identifier: GPL-2.0-or-later
"""Bloodborne launcher GTK application class and CLI play entrypoint."""

import os
import sys
from pathlib import Path
from gi.repository import Adw, Gio
from launcher.gtk.config import load_settings
from launcher.gtk.constants import PORT_DIR
from launcher.gtk.environment import game_environment
from launcher.gtk.window import LauncherWindow


class LauncherApp(Adw.Application):
    """GTK4 application lifecycle manager."""

    def __init__(self):
        super().__init__(
            application_id="io.github.bbport.Launcher",
            flags=Gio.ApplicationFlags.DEFAULT_FLAGS
        )

    def do_activate(self):
        window = self.get_active_window() or LauncherWindow(self)
        window.present()


def play():
    """--play: launch game with saved settings, without showing launcher window."""
    settings = load_settings()
    if not (Path(settings["game_dir"]).expanduser() / "eboot.bin").is_file():
        print("bbport: choose the game folder in the launcher first", file=sys.stderr)
        return 1
    os.chdir(PORT_DIR)
    os.execvpe("bash", ["bash", str(PORT_DIR / "run.sh")], game_environment(settings))
