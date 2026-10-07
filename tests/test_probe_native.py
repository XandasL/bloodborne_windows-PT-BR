# SPDX-License-Identifier: GPL-2.0-or-later
"""Native package and contract validation tests for bb-probe."""

from pathlib import Path
import subprocess
import tempfile
import unittest
from paths import ROOT
from test_probe_pkg import native_package

EXE = ROOT / 'out/bb-probe'


def run_image(data, *options):
    with tempfile.TemporaryDirectory() as tmp:
        path = Path(tmp) / 'boot.bin'
        path.write_bytes(data)
        return subprocess.run([str(EXE.resolve()), str(path), '--cpu-only', *options],
                              capture_output=True, text=True, timeout=5)


@unittest.skipUnless(EXE.exists(), 'build native probe first')
class NativeLoaderTests(unittest.TestCase):
    def test_native_initializer_and_export_return(self):
        r = run_image(native_package())
        self.assertEqual(r.returncode, 20, r.stdout + r.stderr)
        self.assertIn('Module 0 initializer returned 0', r.stdout)
        self.assertIn('first unsupported PS4 import: after-native', r.stdout)

    def test_host_contract_takes_priority_over_native_export(self):
        r = run_image(native_package(name='bzQExy189ZI#q#q', native=b'\x0f\x0b'))
        self.assertEqual(r.returncode, 20, r.stdout + r.stderr)
        self.assertIn('_init_env returned', r.stdout)
        self.assertIn('first unsupported PS4 import: after-native', r.stdout)

    def test_strict_mode_skips_native_initialization_and_binding(self):
        r = run_image(native_package(init=b'\x0f\x0b'), '--strict-imports')
        self.assertEqual(r.returncode, 20, r.stdout + r.stderr)
        self.assertIn('first unsupported PS4 import: fixture-native', r.stdout)
        self.assertNotIn('Starting native libc', r.stdout)

    def test_native_metadata_and_bindings_are_validated(self):
        cases = [
            ({'binding_address': 12288}, 'invalid native binding'),
            ({'binding_kind': 2}, 'native export kind/range mismatch'),
            ({'lib_flags': 4}, 'native function is not executable'),
            ({'metadata': (4096, 8192, 4096, 8192, 32, 33, 1, 256)}, 'invalid linked module metadata'),
            ({'metadata': (4096, 8192, 4096, 8192, 32, 4, 1, 12280)}, 'unmapped procparam'),
        ]
        for kwargs, msg in cases:
            with self.subTest(kwargs=kwargs):
                r = run_image(native_package(**kwargs))
                self.assertEqual(r.returncode, 1, r.stdout + r.stderr)
                self.assertIn(msg, r.stderr)

    def test_failed_native_initializer_does_not_enter_game(self):
        r = run_image(native_package(init=b'\xb8\x01\0\0\0\xc3'))
        self.assertEqual(r.returncode, 1, r.stdout + r.stderr)
        self.assertIn('module initializer failed', r.stderr)
        self.assertNotIn('Entering original', r.stdout)
