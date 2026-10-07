# SPDX-License-Identifier: GPL-2.0-or-later
"""Window layout construction and launch button state."""

from bbport_i18n import tr
from gi.repository import Adw, Gtk
from launcher.gtk.log_view import build_log_page
from launcher.gtk.settings_page import build_settings_page


def build_main_window(win):
    toolbar = Adw.ToolbarView()
    header = Adw.HeaderBar()
    win.stack = Adw.ViewStack()
    switcher = Adw.ViewSwitcher(stack=win.stack, policy=Adw.ViewSwitcherPolicy.WIDE)
    header.set_title_widget(switcher)
    win.launch_button = Gtk.Button()
    win.launch_button.connect("clicked", win.on_launch)
    header.pack_end(win.launch_button)
    toolbar.add_top_bar(header)

    win.toasts = Adw.ToastOverlay()
    win.toasts.set_child(win.stack)
    toolbar.set_content(win.toasts)
    win.set_content(toolbar)

    log_text = win.log_view.get_buffer().get_text(
        *win.log_view.get_buffer().get_bounds(), False
    ) if hasattr(win, "log_view") else ""
    win.stack.add_titled_with_icon(
        build_settings_page(win), "settings", tr("Settings"),
        "preferences-system-symbolic"
    )
    win.stack.add_titled_with_icon(
        build_log_page(win), "log", tr("Log"),
        "utilities-terminal-symbolic"
    )
    win.log_view.get_buffer().set_text(log_text)
    win.update_launch_button()
    win.update_game_status()
    win.update_user_status()
    win.update_upscaler_status()


def update_launch_button(win):
    running = win.process is not None
    win.launch_button.set_label(tr("Stop") if running else tr("Play"))
    win.launch_button.remove_css_class(
        "destructive-action" if not running else "suggested-action"
    )
    win.launch_button.add_css_class(
        "destructive-action" if running else "suggested-action"
    )
