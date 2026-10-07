# SPDX-License-Identifier: GPL-2.0-or-later
"""Vulkan host NVIDIA library and manifest discovery routines."""

import json
from pathlib import Path


def elf64(path):
    """Reject 32-bit ICDs in distributions that install both architectures."""
    try:
        with Path(path).open("rb") as stream:
            header = stream.read(20)
        return header[:6] == b"\x7fELF\x02\x01" and header[18:20] == b"\x3e\x00"
    except OSError:
        return False


def host_nvidia(manifest_dirs, library_dirs):
    """Scan manifests for host NVIDIA ICD installation."""
    for directory in manifest_dirs:
        for manifest in sorted(directory.glob("*nvidia*.json")):
            try:
                data = json.loads(manifest.read_text())
                raw_path = str(data["ICD"]["library_path"])
                library = Path(raw_path)
            except (OSError, ValueError, KeyError, TypeError):
                continue
            if "nvidia" not in library.name.lower():
                continue
            if library.is_absolute() or raw_path.startswith("/"):
                candidates = [library, *(d / library.name for d in library_dirs)]
            elif len(library.parts) > 1:
                candidates = [manifest.parent / library]
            else:
                candidates = [d / library for d in library_dirs]
            for candidate in candidates:
                if elf64(candidate):
                    return data, candidate.resolve()
    return None
