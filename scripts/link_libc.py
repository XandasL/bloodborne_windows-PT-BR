#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Link the user's plaintext libc into a separate, reproducible probe image.

The original boot.bin remains usable. Symbols are matched by library/module
identity, versions and NID, not by dump-local suffix or NID alone.
"""

import argparse
from pathlib import Path
import sys

# Ensure scripts dir is on sys.path
_scripts_dir = Path(__file__).resolve().parent
if str(_scripts_dir) not in sys.path:
    sys.path.insert(0, str(_scripts_dir))

from libc_link import encode_id, module, link  # noqa: E402

__all__ = ["encode_id", "module", "link"]


def main():
    """Command-line interface entry point."""
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("game", type=Path)
    p.add_argument(
        "--out",
        type=Path,
        default=Path(__file__).resolve().parent.parent / "out"
    )
    args = p.parse_args()
    link(args.game, args.out)


if __name__ == "__main__":
    main()
