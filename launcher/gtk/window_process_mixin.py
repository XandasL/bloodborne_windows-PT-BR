# SPDX-License-Identifier: GPL-2.0-or-later
"""Window mixin for process lifecycle, logging, and environment."""

from launcher.gtk import environment, log_view, process_killer, process_runner


class WindowProcessMixin:
    """Methods for execution, logging, and child process lifecycle."""

    def environment(self):
        return environment.game_environment(self.settings)

    def append_log(self, text):
        log_view.append_log(self, text)

    def on_launch(self, button):
        process_runner.on_launch(self, button)

    def read_line(self):
        process_runner.read_line(self)

    def on_line(self, stream, result):
        process_runner.on_line(self, stream, result)

    def stop_game(self):
        process_killer.stop_game(self)

    def kill_if_running(self, pid):
        return process_killer.kill_if_running(self, pid)

    def on_exit(self, process, result):
        process_runner.on_exit(self, process, result)

    def on_close(self, window):
        return process_killer.on_close(self, window)
