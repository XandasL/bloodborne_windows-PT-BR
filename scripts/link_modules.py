#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Link the eboot with the game's bundled system modules into one probe image."""

import argparse
from pathlib import Path
import sys

_scripts_dir = Path(__file__).resolve().parent
if str(_scripts_dir) not in sys.path:
    sys.path.insert(0, str(_scripts_dir))

from guest_modules import (  # noqa: E402
    DEFAULT_MODULES, FS_LOAD, link, module, patch_fs_loads
)

__all__ = ["DEFAULT_MODULES", "FS_LOAD", "link", "module", "patch_fs_loads"]


def main():
    """Command-line entry point."""
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("game", type=Path)
    p.add_argument(
        "--out",
        type=Path,
        default=Path(__file__).resolve().parent.parent / "out"
    )
    p.add_argument("--modules", nargs="*", default=list(DEFAULT_MODULES))
    a = p.parse_args()
    link(a.game, a.out, a.modules)


if __name__ == "__main__":
    main()
