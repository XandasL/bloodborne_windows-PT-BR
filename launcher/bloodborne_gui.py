#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Bloodborne Cross-Platform Launcher & Mod Manager (GUI Entry Point)."""

import sys
from pathlib import Path

# Ensure package roots are accessible
current_dir = Path(__file__).resolve().parent
if str(current_dir) not in sys.path:
    sys.path.insert(0, str(current_dir))

from gui import BloodborneLauncherApp


def main():
    """Start the graphical launcher application."""
    app = BloodborneLauncherApp()
    app.mainloop()


if __name__ == "__main__":
    main()
