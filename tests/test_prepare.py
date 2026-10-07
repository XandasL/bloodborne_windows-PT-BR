# SPDX-License-Identifier: GPL-2.0-or-later
"""Tests for PS4 SELF inspection and entry-point preparation."""

from pathlib import Path
import struct
import tempfile
import unittest
from prepare import inspect_libc, nid, parse_self
from test_prepare_fixtures import make_libc_fixture, make_self_fixture


class SelfTests(unittest.TestCase):
    def test_libc_ret_contract_is_verified_from_symbol_and_code(self):
        with tempfile.TemporaryDirectory() as tmp:
            path = Path(tmp) / 'libc.prx'
            path.write_bytes(make_libc_fixture(0xc3))
            proof = inspect_libc(path)
            self.assertTrue(proof['init_env_is_ret'])
            self.assertEqual(proof['bytes'], 'c3')
            path.write_bytes(make_libc_fixture(0x90))
            self.assertFalse(inspect_libc(path)['init_env_is_ret'])
        self.assertEqual(nid('_init_env'), 'bzQExy189ZI')

    def test_file_offset_is_not_virtual_address(self):
        elf, header, ph, segments, missing = parse_self(make_self_fixture())
        self.assertEqual(elf[0x1000:0x1010], bytes(range(16)))
        self.assertEqual(ph[0]['vaddr'], 0)
        self.assertEqual(missing, [])

    def test_truncated_payload_rejected(self):
        with self.assertRaisesRegex(ValueError, 'out of bounds'):
            parse_self(make_self_fixture()[:-1])

    def test_encrypted_or_compressed_payload_rejected(self):
        for flag in (2, 8):
            data = make_self_fixture()
            struct.pack_into('<Q', data, 32, 0x800 | flag)
            with self.assertRaisesRegex(ValueError, 'encrypted/compressed'):
                parse_self(data)

    def test_missing_required_segment_rejected(self):
        data = make_self_fixture()
        struct.pack_into('<Q', data, 32, 0)
        with self.assertRaisesRegex(ValueError, 'required segment'):
            parse_self(data)

    def test_bad_program_header_index_rejected(self):
        data = make_self_fixture()
        struct.pack_into('<Q', data, 32, 0x100800)
        with self.assertRaisesRegex(ValueError, 'segment index'):
            parse_self(data)


if __name__ == '__main__':
    unittest.main()
