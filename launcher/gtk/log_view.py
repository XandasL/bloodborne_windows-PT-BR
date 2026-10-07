# SPDX-License-Identifier: GPL-2.0-or-later
"""Log page widget and buffer appending utility."""

from gi.repository import GLib, Gtk
from launcher.gtk.constants import MAX_LOG_LINES


def build_log_page(win):
    box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
    win.log_view = Gtk.TextView(
        editable=False, monospace=True, cursor_visible=False,
        wrap_mode=Gtk.WrapMode.WORD_CHAR
    )
    win.log_view.set_top_margin(8)
    win.log_view.set_left_margin(8)
    win.log_view.set_right_margin(8)
    scroller = Gtk.ScrolledWindow(vexpand=True, child=win.log_view)
    win.log_scroller = scroller
    box.append(scroller)
    return box


def append_log(win, text):
    buffer = win.log_view.get_buffer()
    buffer.insert(buffer.get_end_iter(), text)
    extra = buffer.get_line_count() - MAX_LOG_LINES
    if extra > 0:
        buffer.delete(buffer.get_start_iter(), buffer.get_iter_at_line(extra)[1])
    adj = win.log_scroller.get_vadjustment()
    GLib.idle_add(lambda: adj.set_value(adj.get_upper()) and False)
