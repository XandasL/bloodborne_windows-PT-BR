# SPDX-License-Identifier: GPL-2.0-or-later
"""Subprocess spawning, stream polling, and process lifecycle management."""

from bbport_i18n import tr
from gi.repository import Adw, Gio, GLib
from launcher.gtk.constants import PORT_DIR
from launcher.gtk.process_killer import stop_game


def on_launch(win, _button):
    if win.process:
        stop_game(win)
        return
    win.store()
    win.log_view.get_buffer().set_text("")
    flags = Gio.SubprocessFlags.STDOUT_PIPE | Gio.SubprocessFlags.STDERR_MERGE
    launcher = Gio.SubprocessLauncher.new(flags)
    launcher.set_environ([f"{k}={v}" for k, v in win.environment().items()])
    launcher.set_cwd(str(PORT_DIR))
    try:
        win.process = launcher.spawnv(["setsid", "bash", str(PORT_DIR / "run.sh")])
    except GLib.Error as error:
        win.toasts.add_toast(Adw.Toast(title=tr("Could not start: {}").format(error.message)))
        return
    win.stream = Gio.DataInputStream.new(win.process.get_stdout_pipe())
    read_line(win)
    win.process.wait_async(None, lambda p, r: on_exit(win, p, r))
    win.update_launch_button()
    win.stack.set_visible_child_name("log")


def read_line(win):
    win.stream.read_line_async(GLib.PRIORITY_DEFAULT, None, lambda s, r: on_line(win, s, r))


def on_line(win, stream, result):
    try:
        line, _ = stream.read_line_finish_utf8(result)
    except GLib.Error:
        return
    if line is not None:
        win.append_log(line + "\n")
        read_line(win)


def on_exit(win, process, result):
    try:
        process.wait_finish(result)
    except GLib.Error:
        pass
    status = process.get_exit_status() if process.get_if_exited() else -1
    win.process = None
    win.update_launch_button()
    win.update_game_status()
    win.append_log(tr("\n— game exited (code {}) —\n").format(status))
