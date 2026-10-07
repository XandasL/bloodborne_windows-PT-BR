# SPDX-License-Identifier: GPL-2.0-or-later
"""Window mixin for directory paths, UI language, upscaler and settings store."""

from launcher.gtk import status_paths, status_upscaler, store_state


class WindowStatusMixin:
    """Methods for path status, language and settings persistence."""

    def game_dir(self):
        return status_paths.game_dir(self)

    def user_dir(self):
        return status_paths.user_dir(self)

    def update_user_status(self):
        status_paths.update_user_status(self)

    def choose_folder(self, title, current, done):
        status_paths.choose_folder(self, title, current, done)

    def on_choose_user(self, button):
        status_paths.on_choose_user(self, button)

    def on_reset_user(self, button):
        status_paths.on_reset_user(self, button)

    def update_upscaler_status(self):
        status_upscaler.update_upscaler_status(self)

    def update_game_status(self):
        status_paths.update_game_status(self)

    def on_choose_game(self, button):
        status_paths.on_choose_game(self, button)

    def on_ui_language(self, row, param):
        store_state.on_ui_language(self, row, param)

    def store(self):
        store_state.store(self)
