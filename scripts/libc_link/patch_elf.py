# SPDX-License-Identifier: GPL-2.0-or-later
"""Eboot instruction patching and TLS extraction."""


def patch_fs_to_gs(image, main_ph):
    """Rewrite `mov rax, fs:[0]` instructions to `gs:[0]`."""
    fs_load = bytes.fromhex('64488b042500000000')
    patched = 0
    for p in main_ph:
        if p['type'] != 1 or not p['flags'] & 1:
            continue
        start, end = p['vaddr'], p['vaddr'] + p['filesz']
        at = image.find(fs_load, start, end)
        while at >= 0:
            image[at] = 0x65
            patched += 1
            at = image.find(fs_load, at + len(fs_load), end)
    return patched


def extract_main_tls(main_ph, size):
    """Extract and validate main program TLS metadata."""
    main_tls = next((p for p in main_ph if p['type'] == 7), None)
    if not main_tls:
        return (0, 0, 0, 0)
    values = (main_tls['vaddr'], main_tls['filesz'], main_tls['memsz'], main_tls['align'])
    if (main_tls['filesz'] > main_tls['memsz'] or main_tls['memsz'] > 1024 * 1024
            or main_tls['vaddr'] + main_tls['filesz'] > size):
        raise ValueError('unsupported eboot TLS layout')
    return values
