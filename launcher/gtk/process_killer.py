# SPDX-License-Identifier: GPL-2.0-or-later
"""Subprocess termination and cleanup handlers."""

import os
import signal
from gi.repository import GLib


def stop_game(win):
    if not win.process:
        return
    pid = int(win.process.get_identifier())
    try:
        os.killpg(pid, signal.SIGTERM)
    except OSError:
        win.process.force_exit()
    GLib.timeout_add_seconds(3, lambda: kill_if_running(win, pid))


def kill_if_running(win, pid):
    if win.process:
        try:
            os.killpg(pid, signal.SIGKILL)
        except OSError:
            pass
    return False


def on_close(win, _window):
    win.store()
    if win.process:
        stop_game(win)
    return False
