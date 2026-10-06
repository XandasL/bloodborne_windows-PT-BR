# SPDX-License-Identifier: GPL-2.0-or-later
"""In-process asset preparation pipeline and native engine execution."""

import importlib
import os
from pathlib import Path
import subprocess
import sys


def execute_preparation(game_path, out_dir, fps, config_file, patches_dir):
    """Run preparation passes in-process to avoid recursive process calls."""
    # Pass 1: Prepare ELF image
    prep_mod = importlib.import_module("prepare")
    prep_mod.prepare(Path(game_path), Path(out_dir))

    # Pass 2: Link Libc
    libc_mod = importlib.import_module("link_libc")
    libc_mod.link(Path(game_path), Path(out_dir))

    # Pass 3: Link Guest Modules
    mods_mod = importlib.import_module("link_modules")
    mods_mod.link(Path(game_path), Path(out_dir))

    # Pass 4: Content Profile
    prof_mod = importlib.import_module("content_profile")
    prof_mod.prepare(Path(game_path), Path(out_dir), sku="full")

    # Pass 5: Engine Patches
    patch_mod = importlib.import_module("patches")
    xml_path = Path(patches_dir) / "Bloodborne.xml"

    old_argv = sys.argv
    try:
        sys.argv = [
            "patches.py",
            "--out", str(out_dir),
            "--fps", str(fps),
            "--game-dir", str(game_path),
            "--settings", str(config_file),
            "--xml", str(xml_path)
        ]
        patch_mod.main()
    finally:
        sys.argv = old_argv


def spawn_game_process(probe_exe, game_dir, config_file, base_dir):
    """Spawn the native bb-probe game binary."""
    env = os.environ.copy()
    env["BB_GAME_DIR"] = str(game_dir)
    env["BB_CONFIG"] = str(config_file)
    env["PATH"] = str(base_dir) + os.pathsep + env.get("PATH", "")

    return subprocess.Popen(
        [str(probe_exe)],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        cwd=str(base_dir),
        env=env
    )
