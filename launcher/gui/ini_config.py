# SPDX-License-Identifier: GPL-2.0-or-later
"""bbport.ini configuration serialization and state management."""

from pathlib import Path


def save_bbport_settings(config_path, settings_dict):
    """Update or append key-value pairs in bbport.ini."""
    path = Path(config_path)
    lines = []
    if path.is_file():
        lines = path.read_text(encoding="utf-8", errors="ignore").splitlines()

    existing_keys = set()
    new_lines = []
    for line in lines:
        if "=" in line and not line.strip().startswith(("#", ";")):
            k, _ = line.split("=", 1)
            k = k.strip()
            if k in settings_dict:
                new_lines.append(f"{k} = {settings_dict[k]}")
                existing_keys.add(k)
                continue
        new_lines.append(line)

    for k, v in settings_dict.items():
        if k not in existing_keys:
            new_lines.append(f"{k} = {v}")

    path.write_text("\n".join(new_lines) + "\n", encoding="utf-8")


def build_settings_payload(app):
    """Extract settings dictionary from application Tk variables."""
    return {
        "upscaler": app.upscaler_var.get(),
        "preset": app.preset_var.get(),
        "output_res": app.resolution_var.get(),
        "show_fps": "1" if app.show_fps_var.get() else "0",
        "skip_intro": "1" if app.skip_intro_var.get() else "0",
        "effect_motion_blur": "1" if app.motion_blur_var.get() else "0",
        "effect_dof": "1" if app.dof_var.get() else "0",
        "effect_chromatic_aberration": "1" if app.chromatic_aberration_var.get() else "0",
        "effect_ssao": "1" if app.ssao_var.get() else "0",
        "effect_ssr": "1" if app.ssr_var.get() else "0"
    }
