# SPDX-License-Identifier: GPL-2.0-or-later
"""Mod files scanner and filesystem expansion."""

import os
from pathlib import Path
from .discovery import content_root


def mod_files(folder):
    """(game path, source) of every file a mod replaces or adds under dvdroot_ps4."""
    layout = content_root(folder)
    if not layout:
        raise ValueError(f'{folder}: expected dvdroot_ps4 inside the mod folder')
    root, prefix = layout
    root = root.resolve()
    for directory, folders, files in os.walk(root, followlinks=False):
        dir_p = Path(directory)
        for name in [*folders, *files]:
            if (dir_p / name).is_symlink():
                raise ValueError(f'Mod symlinks are unsupported: {dir_p / name}')
        for name in sorted(files):
            source = dir_p / name
            relative = Path(prefix) / source.relative_to(root) if prefix else source.relative_to(root)
            if relative.parts[0].casefold() != 'dvdroot_ps4':
                if relative.parts[0].casefold() in ('eboot.bin', 'sce_module', 'sce_sys'):
                    raise ValueError(f'Executable/system replacement is unsupported: {source}')
                continue
            if not source.is_file():
                raise ValueError(f'Not a regular mod file: {source}')
            yield relative, source


def expand(directory):
    """Materialize one directory level; never write through a directory link."""
    if directory.is_symlink():
        target = directory.resolve(strict=True)
        if not target.is_dir():
            raise ValueError(f'File/directory conflict at {directory.name}')
        directory.unlink()
        directory.mkdir()
        for entry in target.iterdir():
            (directory / entry.name).symlink_to(entry, target_is_directory=entry.is_dir())
    elif directory.exists() and not directory.is_dir():
        raise ValueError(f'File/directory conflict at {directory.name}')
    else:
        directory.mkdir(exist_ok=True)
