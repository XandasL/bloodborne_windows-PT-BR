#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Inspect an already plaintext PS4 SELF; prepare a native entry-point probe.

No decryption, patches to source files, third-party modules or Python dependencies.
Format references are linked in README.md. All offsets are checked before use.
"""

import argparse
from pathlib import Path
import sys

_scripts_dir = Path(__file__).resolve().parent
if str(_scripts_dir) not in sys.path:
    sys.path.insert(0, str(_scripts_dir))

from elf_prepare import (  # noqa: E402
    inspect_libc, nid, parse_self, prepare, sfo, span, unpack
)

__all__ = ["inspect_libc", "nid", "parse_self", "prepare", "sfo", "span", "unpack"]


def main():
    """Command-line entry point."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("game", type=Path)
    parser.add_argument(
        "--out",
        type=Path,
        default=Path(__file__).resolve().parent.parent / "out"
    )
    args = parser.parse_args()
    try:
        prepare(args.game, args.out)
    except (ValueError, OSError, StopIteration, KeyError, IndexError) as error:
        parser.exit(1, f"prepare failed: {error}\n")


if __name__ == "__main__":
    main()
