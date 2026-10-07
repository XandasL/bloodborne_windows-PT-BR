# SPDX-License-Identifier: GPL-2.0-or-later
"""Window mixin for mod and patch discovery, list ordering, and persistence."""

from launcher.gtk import mods_manager, patches_manager


class WindowModsMixin:
    """Methods for mods and third-party patches."""

    def mods_dir(self):
        return mods_manager.mods_dir(self)

    def save_mod_profile(self):
        mods_manager.save_mod_profile(self)

    def refresh_mods(self):
        mods_manager.refresh_mods(self)

    def move_mod(self, name, direction):
        mods_manager.move_mod(self, name, direction)

    def on_refresh_mods(self, button):
        mods_manager.on_refresh_mods(self, button)

    def on_choose_mods(self, button):
        mods_manager.on_choose_mods(self, button)

    def save_patch_profile(self):
        patches_manager.save_patch_profile(self)

    def refresh_patches(self):
        patches_manager.refresh_patches(self)

    def on_refresh_patches(self, button):
        patches_manager.on_refresh_patches(self, button)

    def on_choose_patches(self, button):
        patches_manager.on_choose_patches(self, button)
