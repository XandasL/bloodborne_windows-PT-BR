# SPDX-License-Identifier: GPL-2.0-or-later
"""Exercise run.sh across a re-exec with lightweight preparation/probe stand-ins."""

import unittest
from test_run_settings_helper import run_restarts


class RestartResolutionTests(unittest.TestCase):
    def test_outputs_other_than_1080p_patch_the_render_size_and_restarts_recompute_it(self):
        rows = run_restarts()
        self.assertEqual([row['BB_RENDER_RES'] for row in rows], ['854x480', '426x240', None])
        self.assertEqual([row['BB_OUTPUT_RES'] for row in rows], ['1280x720', '1280x720', None])
        self.assertEqual([row['BB_AUTO_RENDER_RES'] for row in rows], ['1', '1', None])

    def test_live_resolution_keeps_guest_sizes_native(self):
        rows = run_restarts(live=True)
        self.assertTrue(all(row['BB_RENDER_RES'] is None for row in rows))
        self.assertTrue(all(row['BB_OUTPUT_RES'] is None for row in rows))
        self.assertTrue(all(row['BB_AUTO_RENDER_RES'] is None for row in rows))

    def test_live_resolution_setting_and_gpu_check(self):
        self.assertIsNone(run_restarts(ini_extra='live_resolution=1\n')[0]['BB_RENDER_RES'])
        self.assertEqual(run_restarts(ini_extra='live_resolution=0\n', caps=1)[0]['BB_RENDER_RES'], '854x480')
        self.assertEqual(run_restarts(caps=1)[0]['BB_RENDER_RES'], '854x480')
        self.assertIsNone(run_restarts(ini_extra='live_resolution=auto\n', caps=1)[0]['BB_RENDER_RES'])
        self.assertEqual(run_restarts(ini_extra='live_resolution=auto\n', caps=0)[0]['BB_RENDER_RES'], '854x480')

    def test_live_resolution_without_sed_or_grep(self):
        self.assertEqual(run_restarts(ini_extra='live_resolution=0\n', caps=1,
                                     bare_path=True)[0]['BB_RENDER_RES'], '854x480')
        self.assertIsNone(run_restarts(ini_extra='live_resolution=auto\n', caps=1,
                                      bare_path=True)[0]['BB_RENDER_RES'])

    def test_explicit_render_override_survives_restart(self):
        rows = run_restarts(explicit=True)
        self.assertEqual([row['BB_RENDER_RES'] for row in rows], ['800x450'] * 3)
        self.assertTrue(all(row['BB_AUTO_RENDER_RES'] is None for row in rows))
