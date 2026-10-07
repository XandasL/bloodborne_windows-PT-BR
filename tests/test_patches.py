# SPDX-License-Identifier: GPL-2.0-or-later
"""Tests for debug and third-party external XML patch management."""

from pathlib import Path
import tempfile
import unittest
from patches import (
    compile_external, compile_patches, effect_patches, external_patches,
    external_selection, validate_patch_requirements
)
from test_patches_helpers import EXTERNAL_XML, SEGMENTS, XML


class DebugPatchTests(unittest.TestCase):
    def test_camera_patch_is_optional_and_compatible_with_fps_and_debug_menu(self):
        self.assertEqual(effect_patches({'debug_camera': '0', 'debug_menu': '0'}), [])
        camera = effect_patches({'debug_camera': '1'})
        writes = compile_patches(XML, camera, '01.09', SEGMENTS)
        self.assertGreater(len(writes), 0)
        camera_bytes = {offset + i: byte for offset, data in writes for i, byte in enumerate(data)}
        for patch in ('Uncap FPS++', '60 FPS++', '90 FPS++', 'Restore Debug Menu (READ NOTES)'):
            for offset, data in compile_patches(XML, [patch], '01.09', SEGMENTS):
                for i, byte in enumerate(data):
                    if offset + i in camera_bytes:
                        self.assertEqual(camera_bytes[offset + i], byte, patch)

    def test_debug_menu_checks_both_fonts_but_camera_does_not_need_them(self):
        names = effect_patches({'debug_menu': '1'})
        with tempfile.TemporaryDirectory() as directory:
            game = Path(directory)
            validate_patch_requirements(['Restore Debug Camera'], game)
            with self.assertRaisesRegex(ValueError, 'DbgFont14h.ccm.*DbgFont14h.tpf'):
                validate_patch_requirements(names, game)
            font = game / 'dvdroot_ps4/font'
            font.mkdir(parents=True)
            (font / 'DbgFont14h.ccm').write_bytes(b'test')
            (font / 'DbgFont14h.tpf').touch()
            with self.assertRaisesRegex(ValueError, 'DbgFont14h.tpf'):
                validate_patch_requirements(names, game)
            (font / 'DbgFont14h.tpf').write_bytes(b'test')
            validate_patch_requirements(names, game)

    def test_conflicting_enemy_control_patch_is_rejected(self):
        with self.assertRaisesRegex(ValueError, 'conflicts with Enemy Control'):
            validate_patch_requirements(['Enemy Control', 'Restore Debug Camera'], Path('.'))


class ExternalPatchTests(unittest.TestCase):
    def test_selection_and_unsupported_lines(self):
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory)
            (folder / 'extra.xml').write_text(EXTERNAL_XML)
            (folder / 'other.xml').write_text(EXTERNAL_XML.replace('CUSA03173', 'CUSA99999').replace('CUSA00207', 'CUSA99998'))
            (folder / 'broken.xml').write_text('<Patch>')
            found = external_patches(folder)
            self.assertEqual([key for key, _, _ in found], ['extra.xml/On', 'extra.xml/Off', 'extra.xml/Mask'])
            writes = compile_external(external_selection(found, None), SEGMENTS)
            self.assertEqual(writes, [(0x1000, bytes.fromhex('9090'))])
            config = folder / 'patches.json'
            config.write_text('{"enabled": ["extra.xml/Off"], "disabled": ["extra.xml/On"]}')
            writes = compile_external(external_selection(found, config), SEGMENTS)
            self.assertEqual(writes, [(0x2000, (0x12345678).to_bytes(4, 'little'))])

    def test_built_in_file_is_not_external(self):
        self.assertEqual(external_patches(XML.parent), [])


if __name__ == '__main__':
    unittest.main()
