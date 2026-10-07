# SPDX-License-Identifier: GPL-2.0-or-later
"""Cross-module identity and TLS relocation tests without game binaries."""

import struct
import unittest
from test_link_libc_helpers import make_fixture, run_link


class LinkTests(unittest.TestCase):
    def test_different_local_ids_bind_same_identity(self):
        data, report = run_link(*make_fixture())
        self.assertEqual(data[:8], b'BBPROBE4')
        self.assertEqual(report['symbol_bindings'], [
            dict(import_name='fixture#q#q', address='0x10010', kind=1)])

    def test_same_nid_wrong_library_version_does_not_bind(self):
        _, report = run_link(*make_fixture(version=2))
        self.assertEqual(report['bindings'], 0)

    def test_tls_module_id_is_literal_not_image_pointer(self):
        data, report = run_link(*make_fixture(relocs=[(64, 16, 0, 0)]))
        size = struct.unpack_from('<Q', data, 8)[0]
        image = data[-size:]
        self.assertEqual(struct.unpack_from('<Q', image, 65536 + 64)[0], 2)
        self.assertEqual(report['tls_relocations'], 1)

    def test_unsupported_tls_symbol_rejected(self):
        with self.assertRaisesRegex(ValueError, 'unsupported external TLS'):
            run_link(*make_fixture(relocs=[(64, 16, 1, 0)]))

    def test_relocation_outside_segments_rejected(self):
        with self.assertRaisesRegex(ValueError, 'relocation outside image'):
            run_link(*make_fixture(relocs=[(4092, 8, 0, 0)]))

    def test_fs_thread_pointer_loads_are_rewritten_to_gs(self):
        main, libc = make_fixture()
        load = bytes.fromhex('64488b042500000000')
        code = load + b'\x90' + load + b'\xc3'
        main['ph'] = main['ph'] + [dict(type=1, vaddr=0, filesz=64, memsz=64, flags=5),
                                    dict(type=7, vaddr=2048, filesz=0, memsz=48, align=16)]
        data, report = run_link(main, libc, code)
        size = struct.unpack_from('<Q', data, 8)[0]
        image = data[-size:]
        self.assertEqual(image[0], 0x65)
        self.assertEqual(image[10], 0x65)
        self.assertEqual(report['fs_loads_patched'], 2)
        self.assertEqual(report['main_tls'], dict(vaddr=2048, filesz=0, memsz=48, align=16))
        self.assertEqual(struct.unpack_from('<4Q', data, 56 + 64), (2048, 0, 48, 16))

    def test_fs_loads_outside_executable_segments_are_kept(self):
        main, libc = make_fixture()
        load = bytes.fromhex('64488b042500000000')
        main['ph'] = main['ph'] + [dict(type=1, vaddr=0, filesz=64, memsz=64, flags=6)]
        data, report = run_link(main, libc, load)
        image = data[-struct.unpack_from('<Q', data, 8)[0]:]
        self.assertEqual(image[0], 0x64)
        self.assertEqual(report['fs_loads_patched'], 0)
