#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Rewrite FSR 4.1.1 postpass SPIR-V image stores through workgroup memory."""

from pathlib import Path
import sys

_dir = Path(__file__).resolve().parent
if str(_dir) not in sys.path:
    sys.path.insert(0, str(_dir))

from postpass.transformer import rewrite_spirv  # noqa: E402


def main():
    lines = sys.stdin.read().split('\n')
    print(rewrite_spirv(lines))


if __name__ == '__main__':
    main()
