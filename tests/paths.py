"""Repository paths for the Python tests (run: python3 -m unittest discover -s tests)."""
import os
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
if str(ROOT / 'scripts') not in sys.path:
    sys.path.insert(0, str(ROOT / 'scripts'))


def _find_bash():
    for p in (r"C:\Program Files\Git\bin\bash.exe", r"C:\Program Files\Git\usr\bin\bash.exe"):
        if Path(p).is_file():
            return p
    return shutil.which('bash') or 'bash'


BASH = _find_bash()
