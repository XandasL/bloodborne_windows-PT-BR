# SPDX-License-Identifier: GPL-2.0-or-later
"""Constants for guest module linking."""

DEFAULT_MODULES = ('libc.prx', 'libSceFios2.prx')
FS_LOAD = bytes.fromhex('64488b042500000000')  # mov rax, fs:[0]
