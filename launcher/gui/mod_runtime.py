# SPDX-License-Identifier: GPL-2.0-or-later
"""Bridge launcher mod selections to the loose-file overlay system."""

import contextlib
import io
from pathlib import Path
import shutil

from .paths import MODS_CONFIG, MODS_DIR
from mod_manager import build_overlay, child, selected


def prepare_modded_game(game_dir, out_dir, mods_dir=MODS_DIR, config_file=MODS_CONFIG):
    """Build the same temporary mod view used by scripts/mods.py."""
    game = Path(game_dir).resolve()
    mods_root = Path(mods_dir)
    layers = []

    legacy = Path(str(game) + "-mods")
    if legacy.is_dir():
        layers.append((legacy.name, legacy))

    if child(mods_root, "dvdroot_ps4").is_dir():
        layers.append((mods_root.name, mods_root))
    else:
        for name in selected(mods_root, config_file):
            layers.append((name, mods_root / name))

    log = io.StringIO()
    try:
        with contextlib.redirect_stderr(log):
            view = Path(build_overlay(game, out_dir, layers))
    except OSError as error:
        if getattr(error, "winerror", None) == 1314:
            raise RuntimeError(
                "Windows blocked creation of the mod overlay. Enable Developer Mode "
                "or allow symbolic-link creation, then try again."
            ) from error
        raise

    messages = [line for line in log.getvalue().splitlines() if line.strip()]
    overlay = view if view != game else None
    return view, overlay, messages


def cleanup_modded_game(overlay):
    """Remove a temporary mod view without touching the original game or mods."""
    if overlay and Path(overlay).exists():
        shutil.rmtree(overlay, ignore_errors=True)
