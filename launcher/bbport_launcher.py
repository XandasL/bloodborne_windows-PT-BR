#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Bloodborne launcher entrypoint (GTK4 / libadwaita with Windows fallback)."""

import sys
from pathlib import Path

PORT_DIR = Path(__file__).resolve().parent.parent
if str(PORT_DIR) not in sys.path:
    sys.path.insert(0, str(PORT_DIR))
if str(PORT_DIR / "scripts") not in sys.path:
    sys.path.insert(0, str(PORT_DIR / "scripts"))
if str(PORT_DIR / "launcher") not in sys.path:
    sys.path.insert(0, str(PORT_DIR / "launcher"))

try:
    import gi
    gi.require_version("Gtk", "4.0")
    gi.require_version("Adw", "1")
    from launcher.gtk import LauncherApp, play
    _HAS_GTK = True
except (ImportError, ValueError):
    _HAS_GTK = False


def main():
    if _HAS_GTK:
        if "--play" in sys.argv[1:]:
            return play()
        return LauncherApp().run(sys.argv)
    from launcher.bloodborne_gui import main as gui_main
    return gui_main()


if __name__ == "__main__":
    sys.exit(main())
