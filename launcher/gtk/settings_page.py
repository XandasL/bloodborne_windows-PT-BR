# SPDX-License-Identifier: GPL-2.0-or-later
"""Assembles all settings sections into the Adw.PreferencesPage."""

from gi.repository import Adw
from launcher.gtk.settings_sections_basic import (
    build_game_group, build_launcher_group
)
from launcher.gtk.settings_sections_mods import (
    build_mods_group, build_patches_group
)
from launcher.gtk.settings_sections_graphics import (
    build_display_group, build_upscaler_group
)
from launcher.gtk.settings_sections_effects import (
    build_effects_group, build_frames_group
)
from launcher.gtk.settings_sections_dev import (
    build_perf_and_dev_groups
)


def build_settings_page(win):
    page = Adw.PreferencesPage()
    page.add(build_launcher_group(win))
    page.add(build_game_group(win))
    page.add(build_mods_group(win))
    page.add(build_patches_group(win))
    page.add(build_display_group(win))
    page.add(build_upscaler_group(win))
    page.add(build_effects_group(win))
    page.add(build_frames_group(win))
    perf, dev = build_perf_and_dev_groups(win)
    page.add(perf)
    page.add(dev)
    return page
