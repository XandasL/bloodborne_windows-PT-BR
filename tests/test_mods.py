# SPDX-License-Identifier: GPL-2.0-or-later
"""Tests for loose-file mod overlay merging."""

import json
from pathlib import Path
import subprocess
import sys
from paths import ROOT
from test_mods_base import BaseModTestCase, mods


class ModTests(BaseModTestCase):
    def test_merge_replacement_new_file_and_directory_listing(self):
        a = self.mod('A')
        self.mod('A', b'new', 'new.dcx')
        result = mods.build_overlay(self.game, self.root / 'out', [('A', a)])
        folder = result / 'dvdroot_ps4' / 'chr'
        self.assertEqual((folder / 'a.dcx').read_bytes(), b'mod')
        self.assertEqual((folder / 'b.dcx').read_bytes(), b'untouched')
        self.assertEqual((folder / 'new.dcx').read_bytes(), b'new')
        self.assertEqual({p.name for p in folder.iterdir()}, {'a.dcx', 'b.dcx', 'new.dcx'})
        self.assertEqual((self.assets / 'a.dcx').read_bytes(), b'original')
        self.assertFalse((self.assets / 'new.dcx').exists())
        self.assertEqual((result / 'eboot.bin').read_bytes(), b'original executable')

    def test_last_mod_wins_and_reordering_changes_winner(self):
        a, b = self.mod('A', b'A'), self.mod('B', b'B')
        for layers, winner in [([('A', a), ('B', b)], b'B'), ([('B', b), ('A', a)], b'A')]:
            result = mods.build_overlay(self.game, self.root / 'out', layers)
            self.assertEqual((result / 'dvdroot_ps4/chr/a.dcx').read_bytes(), winner)

    def test_selection_disable_and_new_mod(self):
        for name in ['C', 'B', 'A']:
            self.mod(name)
        config = self.root / 'mods.json'
        config.write_text(json.dumps({'order': ['B', 'A', 'deleted'], 'disabled': ['A']}))
        self.assertEqual(mods.selected(self.moddir, config), ['B', 'C'])

    def test_no_mod_returns_original_game(self):
        self.assertEqual(mods.build_overlay(self.game, self.root / 'out', []), self.game)
        self.assertFalse((self.root / 'out').exists())

    def test_directory_conflict_does_not_modify_base(self):
        a = self.mod('A')
        (a / 'dvdroot_ps4/chr/a.dcx').unlink()
        nested = a / 'dvdroot_ps4/chr/b.dcx'
        nested.mkdir()
        (nested / 'child').write_bytes(b'bad')
        with self.assertRaises(ValueError):
            mods.build_overlay(self.game, self.root / 'out', [('A', a)])
        self.assertEqual((self.assets / 'b.dcx').read_bytes(), b'untouched')
        self.assertEqual(list((self.root / 'out').iterdir()), [])

    def test_mod_symlinks_rejected(self):
        a = self.mod('A')
        (a / 'dvdroot_ps4/escape').symlink_to(self.assets, target_is_directory=True)
        with self.assertRaises(ValueError):
            mods.build_overlay(self.game, self.root / 'out', [('A', a)])

    def test_executable_replacement_rejected(self):
        a = self.mod('A')
        (a / 'eboot.bin').write_bytes(b'unsupported')
        with self.assertRaises(ValueError):
            mods.build_overlay(self.game, self.root / 'out', [('A', a)])

    def test_shadps4_sibling_folder_and_global_disable(self):
        legacy = Path(str(self.game) + '-mods')
        folder = legacy / 'dvdroot_ps4/chr'
        folder.mkdir(parents=True)
        (folder / 'a.dcx').write_bytes(b'legacy')
        for enabled, expected in [('1', b'legacy'), ('0', b'original')]:
            result = subprocess.run([sys.executable, str(ROOT / 'scripts/mods.py'), str(self.game),
                '--out', str(self.root / 'out'), '--mods-dir', str(self.moddir), '--enabled', enabled],
                capture_output=True, text=True, check=True)
            self.assertEqual((Path(result.stdout.strip()) / 'dvdroot_ps4/chr/a.dcx').read_bytes(), expected)
