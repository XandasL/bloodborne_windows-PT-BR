# SPDX-License-Identifier: GPL-2.0-or-later
"""In-process asset preparation pipeline and native engine execution."""

import importlib
import os
from pathlib import Path
import subprocess
import sys


def execute_preparation(game_path, out_dir, fps, config_file, patches_dir):
    """Run preparation passes in-process to avoid recursive process calls."""
    prep_mod = importlib.import_module("prepare")
    prep_mod.prepare(Path(game_path), Path(out_dir))

    libc_mod = importlib.import_module("link_libc")
    libc_mod.link(Path(game_path), Path(out_dir))

    modules_mod = importlib.import_module("link_modules")
    modules_mod.link(Path(game_path), Path(out_dir))

    profile_mod = importlib.import_module("content_profile")
    profile_mod.prepare(Path(game_path), Path(out_dir), sku="full")

    patch_mod = importlib.import_module("patches")
    xml_path = Path(patches_dir) / "Bloodborne.xml"
    old_argv = sys.argv
    try:
        sys.argv = [
            "patches.py", "--out", str(out_dir), "--fps", str(fps),
            "--game-dir", str(game_path), "--settings", str(config_file),
            "--xml", str(xml_path),
        ]
        patch_mod.main()
    finally:
        sys.argv = old_argv


def build_probe_command(probe_exe, game_dir, out_dir, user_dir):
    """Build the bb-probe command line used by the native runner."""
    out = Path(out_dir)
    return [
        str(probe_exe), str(out / "boot-linked.bin"),
        "--content-profile", str(out / "content.bin"),
        "--patches", str(out / "patches.bin"),
        "--app0", str(game_dir), "--user", str(user_dir),
    ]


def spawn_game_process(probe_exe, game_dir, config_file, base_dir, out_dir):
    """Spawn bb-probe with the prepared image and selected app0 view."""
    env = os.environ.copy()
    env["BB_GAME_DIR"] = str(game_dir)
    env["BB_CONFIG"] = str(config_file)
    env["PATH"] = str(base_dir) + os.pathsep + env.get("PATH", "")
    cmd = build_probe_command(probe_exe, game_dir, out_dir, Path(base_dir) / "user")
    return subprocess.Popen(
        cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
        text=True, cwd=str(base_dir), env=env
    )
