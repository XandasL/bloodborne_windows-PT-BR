# SPDX-License-Identifier: GPL-2.0-or-later
"""Base fixture and setup for mod manager tests."""

import importlib.util
from pathlib import Path
import tempfile
import unittest
from paths import ROOT

spec = importlib.util.spec_from_file_location('bbmods', ROOT / 'scripts/mods.py')
mods = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mods)


class BaseModTestCase(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.game = self.root / 'CUSA03173'
        self.assets = self.game / 'dvdroot_ps4' / 'chr'
        self.assets.mkdir(parents=True)
        (self.game / 'eboot.bin').write_bytes(b'original executable')
        (self.assets / 'a.dcx').write_bytes(b'original')
        (self.assets / 'b.dcx').write_bytes(b'untouched')
        self.moddir = self.root / 'mods'

    def mod(self, name, contents=b'mod', file='a.dcx'):
        root = self.moddir / name
        folder = root / 'dvdroot_ps4' / 'chr'
        folder.mkdir(parents=True, exist_ok=True)
        (folder / file).write_bytes(contents)
        return root
