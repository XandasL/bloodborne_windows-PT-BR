# SPDX-License-Identifier: GPL-2.0-or-later
"""Mod overlay creation and link generation."""

from pathlib import Path
import shutil
import sys
import tempfile
from .discovery import child
from .scanner import expand, mod_files


def collect_replacements(mods):
    """Collect file replacements from all active mods."""
    replacements, owners = [], {}
    for name, root in mods:
        count = 0
        for relative, source in mod_files(root):
            key = relative.as_posix().casefold()
            if key in owners:
                print(f'Mods: {relative}: {owners[key]} -> {name}', file=sys.stderr)
            owners[key] = name
            replacements.append((relative, source))
            count += 1
        print(f'Mods: {name}: {count} files', file=sys.stderr)
    return replacements


def build_overlay(game, out, mods):
    """Construct ephemeral symlink overlay directory with active mods applied."""
    game = Path(game).resolve(strict=True)
    replacements = collect_replacements(mods)
    if not replacements:
        return game
    out = Path(out).resolve()
    out.mkdir(parents=True, exist_ok=True)
    overlay = Path(tempfile.mkdtemp(prefix='mod-game-', dir=out))
    try:
        for entry in game.iterdir():
            (overlay / entry.name).symlink_to(entry, target_is_directory=entry.is_dir())
        replaced = added = 0
        for relative, source in replacements:
            parent = overlay
            for part in relative.parts[:-1]:
                parent = child(parent, part)
                expand(parent)
            destination = child(parent, relative.parts[-1])
            if destination.is_symlink():
                replaced += destination.resolve().is_relative_to(game)
                destination.unlink()
            elif destination.exists():
                raise ValueError(f'File/directory conflict: {relative}')
            else:
                added += 1
            destination.symlink_to(source)
        print(f'Mods: {replaced} game files replaced, {added} added', file=sys.stderr)
        return overlay
    except BaseException:
        shutil.rmtree(overlay)
        raise
