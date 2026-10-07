# SPDX-License-Identifier: GPL-2.0-or-later
"""Configuration and settings persistence for GTK launcher."""

import json
import os
from pathlib import Path
from launcher.gtk.constants import (
    CONFIG_DIR, CONFIG_FILE, DATA_DIR, DEFAULTS, INI_DEFAULTS
)


def load_settings():
    settings = dict(DEFAULTS)
    try:
        settings.update(json.loads(CONFIG_FILE.read_text()))
    except (OSError, ValueError):
        pass
    return settings


def save_settings(settings):
    CONFIG_DIR.mkdir(parents=True, exist_ok=True)
    CONFIG_FILE.write_text(json.dumps(settings, indent=2, ensure_ascii=False))


def ini_path():
    return Path(os.environ.get("BB_CONFIG", DATA_DIR / "bbport.ini"))


def load_ini():
    values = dict(INI_DEFAULTS)
    lines = []
    try:
        lines = ini_path().read_text().splitlines()
    except OSError:
        pass
    for line in lines:
        if "=" in line and not line.lstrip().startswith("#"):
            key, value = line.split("=", 1)
            values[key.strip()] = value.strip()
    return values, lines


def save_ini(values, lines):
    """Rewrites edited keys in place, appends missing ones, keeps comments."""
    written, out = set(), []
    for line in lines:
        if "=" in line and not line.lstrip().startswith("#"):
            key = line.split("=", 1)[0].strip()
            if key in values:
                out.append(f"{key}={values[key]}")
                written.add(key)
                continue
        out.append(line)
    if not lines:
        out.append("# bbport settings (in-game menu: Insert / L3+R3)")
    for key, value in values.items():
        if key not in written:
            out.append(f"{key}={value}")
    ini_path().write_text("\n".join(out) + "\n")


def patches_dir(settings):
    return Path(settings.get("patches_dir") or DATA_DIR / "patches").expanduser()
