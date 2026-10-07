# SPDX-License-Identifier: GPL-2.0-or-later
"""Boundary tests for the native loader; uses tiny synthetic x86-64 images."""

from pathlib import Path
import struct
import subprocess
import tempfile
import unittest
from paths import ROOT
from test_probe_pkg import package

__all__ = ['package']
EXE = ROOT / 'out/bb-probe'


def run_image(data, *options):
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / 'boot.bin'
        path.write_bytes(data)
        return subprocess.run([str(EXE.resolve()), str(path), '--cpu-only', *options],
                              capture_output=True, text=True, timeout=5)


@unittest.skipUnless(EXE.exists(), 'build native probe first')
class LoaderTests(unittest.TestCase):
    def test_invalid_content_profile_is_rejected(self):
        with tempfile.TemporaryDirectory() as tmp:
            p = Path(tmp) / 'content.bin'
            for data in (b'BBCONT01', struct.pack('<8s5I', b'BBCONT01', 2, 0, 0, 0, 0),
                         struct.pack('<8s5I', b'BBCONT01', 3, 0, 0, 0, 0) + b'extra'):
                with self.subTest(data=data):
                    p.write_bytes(data)
                    r = run_image(package(b'\xc3'), '--content-profile', str(p))
                    self.assertEqual(r.returncode, 1, r.stdout + r.stderr)
                    self.assertIn('content profile', r.stderr)

    def test_original_instruction_reaches_named_import(self):
        r = run_image(package(b'\xff\x25\x02\0\0\0\x90\x90', [(8, 1, 0, 0)], ['fixture-import']))
        self.assertEqual(r.returncode, 20, r.stderr)
        self.assertIn('first unsupported PS4 import: fixture-import', r.stdout)

    def test_base_relative_address_is_relocated(self):
        code = b'\xff\x25\x02\0\0\0\x90\x90' + b'\0' * 8 + b'\xff\x25\x02\0\0\0\x90\x90'
        r = run_image(package(code, [(8, 0, 16, 0), (24, 1, 0, 0)], ['after-relative-jump']))
        self.assertEqual(r.returncode, 20, r.stderr)
        self.assertIn('after-relative-jump', r.stdout)

    def test_out_of_bounds_relocation_is_rejected(self):
        self.assertEqual(run_image(package(b'\xc3', [(4092, 0, 0, 0)])).returncode, 1)

    def test_truncated_image_is_rejected(self):
        self.assertEqual(run_image(package(b'\xc3')[:-1]).returncode, 1)

    def test_illegal_instruction_reports_guest_offset(self):
        r = run_image(package(b'\x0f\x0b'))
        self.assertEqual(r.returncode, 132)
        self.assertIn('at guest offset 0x0,', r.stderr)

    def test_runtime_returns_from_verified_init_env(self):
        code = b'\x48\x83\xec\x08\xff\x15\x16\0\0\0\x48\x83\xc4\x08\xff\x25\x14\0\0\0'
        data = package(code, [(32, 1, 0, 0), (40, 1, 1, 0)], ['bzQExy189ZI#q#q', 'after-init'], 1)
        r = run_image(data)
        self.assertEqual(r.returncode, 20, r.stderr)
        self.assertIn('first unsupported PS4 import: after-init', r.stdout)
        strict = run_image(data, '--strict-imports')
        self.assertEqual(strict.returncode, 20, strict.stderr)
        self.assertIn('first unsupported PS4 import: bzQExy189ZI#q#q', strict.stdout)

    def test_unverified_runtime_stays_disabled(self):
        r = run_image(package(b'\xff\x25\x02\0\0\0\x90\x90', [(8, 1, 0, 0)], ['bzQExy189ZI#q#q'], 0))
        self.assertEqual(r.returncode, 20, r.stderr)
        self.assertIn('_init_env=0', r.stdout)

    def test_unknown_capabilities_rejected(self):
        self.assertEqual(run_image(package(b'\xc3', capabilities=2)).returncode, 1)
