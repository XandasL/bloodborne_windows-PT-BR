#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Merge loose-file mods through links, preserving the original game and mod files."""
import argparse
from pathlib import Path
import sys

_scripts_dir = Path(__file__).resolve().parent
if str(_scripts_dir) not in sys.path:
    sys.path.insert(0, str(_scripts_dir))

from mod_manager import (  # noqa: E402
    GAME_FOLDERS, build_overlay, child, content_root, discover, expand,
    mod_files, selected
)

__all__ = [
    "GAME_FOLDERS", "build_overlay", "child", "content_root", "discover",
    "expand", "mod_files", "selected"
]


def main():
    """Command-line entry point."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("game", type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--mods-dir", required=True, type=Path)
    parser.add_argument("--config", type=Path)
    parser.add_argument("--enabled", choices=("0", "1"), default="1")
    args = parser.parse_args()
    layers = []
    if args.enabled == "1":
        legacy = Path(str(args.game.resolve()) + "-mods")
        if legacy.is_dir():
            layers.append((legacy.name, legacy))
        if child(args.mods_dir, "dvdroot_ps4").is_dir():
            layers.append((args.mods_dir.name, args.mods_dir))
        else:
            for name in selected(args.mods_dir, args.config):
                layers.append((name, args.mods_dir / name))
    print(build_overlay(args.game, args.out, layers))


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, TypeError) as error:
        print(f"Mods: {error}", file=sys.stderr)
        sys.exit(1)
