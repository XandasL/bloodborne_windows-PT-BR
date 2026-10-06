# SPDX-License-Identifier: GPL-2.0-or-later
"""Mod installation, discovery, and toggling logic for Bloodborne."""

import json
from pathlib import Path
import zipfile


def get_disabled_mods(config_file):
    """Read the set of disabled mod directory names from mods.json."""
    cfg_path = Path(config_file)
    if not cfg_path.is_file():
        return set()
    try:
        data = json.loads(cfg_path.read_text(encoding="utf-8"))
        return set(data.get("disabled", []))
    except Exception:
        return set()


def save_mods_state(config_file, mods_state_list):
    """Write active/disabled mod configuration to mods.json."""
    disabled = [name for name, var in mods_state_list if not var.get()]
    order = [name for name, _ in mods_state_list]
    payload = {"disabled": disabled, "order": order}
    Path(config_file).write_text(json.dumps(payload, indent=2), encoding="utf-8")
    return len(mods_state_list) - len(disabled), len(disabled)


def extract_mod_archive(archive_path, target_mods_dir):
    """Extract a mod zip package into the mods directory."""
    src = Path(archive_path).resolve()
    if src.suffix.lower() != ".zip":
        raise ValueError("Only .zip mod archives are supported.")
    mod_dir = Path(target_mods_dir) / src.stem
    mod_dir.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(src, "r") as zf:
        zf.extractall(mod_dir)
    return src.stem


def list_installed_mods(mods_dir):
    """Enumerate subdirectories in the mods folder."""
    mdir = Path(mods_dir)
    mdir.mkdir(parents=True, exist_ok=True)
    return sorted([p.name for p in mdir.iterdir() if p.is_dir()])
