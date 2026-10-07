# SPDX-License-Identifier: GPL-2.0-or-later
"""Filesystem paths and auto-discovery utilities for Bloodborne Launcher."""

import os
from pathlib import Path
import sys

# Base directories resolution
if getattr(sys, "frozen", False):
    EXE_DIR = Path(sys.executable).resolve().parent
    BUNDLE_DIR = Path(getattr(sys, "_MEIPASS", EXE_DIR)).resolve()
else:
    EXE_DIR = Path(__file__).resolve().parent.parent.parent
    BUNDLE_DIR = EXE_DIR

BASE_DIR = EXE_DIR
SCRIPTS_DIR = BUNDLE_DIR / "scripts" if (BUNDLE_DIR / "scripts").is_dir() else BASE_DIR / "scripts"
PATCHES_DIR = BUNDLE_DIR / "patches" if (BUNDLE_DIR / "patches").is_dir() else BASE_DIR / "patches"
MODS_DIR = BASE_DIR / "mods"
CONFIG_FILE = BASE_DIR / "bbport.ini"
MODS_CONFIG = BASE_DIR / "mods.json"

if str(SCRIPTS_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPTS_DIR))

CUSA_CANDIDATES = [
    "CUSA00900", "CUSA03173", "CUSA00207", "CUSA00208",
    "CUSA03023", "CUSA01363", "CUSA00299", "CUSA03014"
]


def find_game_dir():
    """Auto-detect game folder in common relative locations."""
    env_dir = os.environ.get("BB_GAME_DIR")
    if env_dir and (Path(env_dir) / "eboot.bin").is_file():
        return str(Path(env_dir).resolve())

    for cusa in CUSA_CANDIDATES:
        for parent in [BASE_DIR, BASE_DIR.parent]:
            cand = parent / cusa
            if (cand / "eboot.bin").is_file():
                return str(cand.resolve())
    return ""


def find_probe_executable():
    """Locate the bb-probe engine binary."""
    cur_exe = Path(sys.executable).resolve()
    candidates = [
        BASE_DIR / "bin" / "bb-probe.exe",
        BASE_DIR / "bin" / "bb-probe",
        BASE_DIR / "build" / "bb-probe.exe",
        BASE_DIR / "build" / "bb-probe",
        BASE_DIR / "out" / "bb-probe.exe",
        BASE_DIR / "bbport.exe",
        BASE_DIR / "Bloodborne.exe",
        BASE_DIR / "bb-probe.exe",
        BASE_DIR / "bb-probe",
    ]
    for cand in candidates:
        if cand.is_file():
            resolved = cand.resolve()
            if getattr(sys, "frozen", False) and resolved == cur_exe:
                continue
            return resolved
    return None

