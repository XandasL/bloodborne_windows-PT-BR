#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Extract and validate FSR 4.1.1 asset sets from capture logs."""

from pathlib import Path
import sys

_dir = Path(__file__).resolve().parent
if str(_dir) not in sys.path:
    sys.path.insert(0, str(_dir))

from extract.analyzer import analyze_captures  # noqa: E402
from extract.compiler import compile_sets  # noqa: E402


def main():
    if len(sys.argv) < 4:
        print("Usage: extract.py <dxil-spirv> <capture root> <output dir>")
        sys.exit(1)
    dxil_spirv, root, out = sys.argv[1:4]
    errors = 0

    def fail(msg):
        nonlocal errors
        errors += 1
        print("MISMATCH", msg)

    sets = analyze_captures(root, fail)
    compile_sets(sets, out, dxil_spirv)
    if errors:
        print(f"{errors} mismatches")
        sys.exit(1)
    print("all rules match the captures")


if __name__ == "__main__":
    main()
