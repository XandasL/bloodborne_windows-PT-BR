# SPDX-License-Identifier: GPL-2.0-or-later
"""Environment preparation for running bloodborne native binary."""

import os
from pathlib import Path
from launcher.gtk.constants import DATA_DIR
from launcher.gtk.config import patches_dir


def game_environment(s):
    """Build environment variables dictionary for run.sh / game process."""
    env = dict(os.environ)
    env["BB_GAME_DIR"] = str(Path(s["game_dir"]).expanduser())
    if s["user_dir"]:
        env["BB_USER_DIR"] = s["user_dir"]
    env["BB_MODS_DIR"] = str(Path(s.get("mods_dir") or DATA_DIR / "mods").expanduser())
    env["BB_MODS_CONFIG"] = str(DATA_DIR / "mods.json")
    env["BB_MODS_ENABLED"] = "1" if s.get("mods_enabled", True) else "0"
    env["BB_PATCHES_DIR"] = str(patches_dir(s))
    env["BB_PATCHES_CONFIG"] = str(DATA_DIR / "patches.json")
    env["BB_LANGUAGE"] = s["language"]
    env["BB_FULLSCREEN"] = "1" if s["fullscreen"] else "0"
    env["BB_PRESENT_MODE"] = s["present_mode"]
    if s["hdr"]:
        env["BB_HDR"] = "1"
    env["BB_FPS"] = s["fps_mode"]
    if s["fps_limit"] > 0:
        env["BB_FPS_LIMIT"] = str(s["fps_limit"])
    if s["draw_pipe"]:
        env["BB_DRAW_PIPE"] = s["draw_pipe"]
    if s["readbacks"]:
        env["BB_READBACKS"] = s["readbacks"]
    if s["mangohud"]:
        env["MANGOHUD"] = "1"
    if s["frame_stats"]:
        env["BB_FRAME_STATS"] = "1"
    if s["gpu_profile"]:
        env["BB_GPU_PROFILE"] = "1"
    if s["vk_validation"]:
        env["BB_VK_VALIDATION"] = "1"
    for item in s["extra_env"].split():
        if "=" in item:
            key, value = item.split("=", 1)
            env[key] = value
    return env
