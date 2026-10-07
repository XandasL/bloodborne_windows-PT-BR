# SPDX-License-Identifier: GPL-2.0-or-later
"""BloodborneLauncherApp main application window class."""

import tkinter as tk
from bbport_i18n import tr
from .paths import find_game_dir
from .styles import setup_theme_styles
from .ui_layout import assemble_ui_components
from .app_events import on_browse_game_dir, on_refresh_game_info, on_load_mods, on_add_mod, on_open_mods_dir
from .actions import run_smoke_test_action, launch_game_action


class BloodborneLauncherApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title(tr("Bloodborne Runner Launcher & Mod Manager"))
        self.geometry("820x680")
        self.minsize(740, 580)
        self.configure(bg="#18181b")

        self.game_dir_var = tk.StringVar(value=find_game_dir())
        self.fps_var = tk.StringVar(value="uncap")
        self.resolution_var = tk.StringVar(value="1920x1080")
        self.upscaler_var = tk.StringVar(value="fsr3")
        self.preset_var = tk.StringVar(value="1")
        self.skip_intro_var = tk.BooleanVar(value=True)
        self.motion_blur_var = tk.BooleanVar(value=True)
        self.dof_var = tk.BooleanVar(value=True)
        self.chromatic_aberration_var = tk.BooleanVar(value=True)
        self.ssao_var = tk.BooleanVar(value=True)
        self.ssr_var = tk.BooleanVar(value=False)
        self.show_fps_var = tk.BooleanVar(value=False)

        self.running_proc, self.mods_data = None, []
        setup_theme_styles(self)
        assemble_ui_components(self)
        on_refresh_game_info(self)
        on_load_mods(self)

    def log(self, text):
        self.after(0, lambda: (self.log_text.insert("end", text + "\n"), self.log_text.see("end")))

    def clear_log(self):
        self.log_text.delete("1.0", "end")

    def add_mod(self):
        on_add_mod(self)

    def load_mods(self):
        on_load_mods(self)

    def open_mods_dir(self):
        on_open_mods_dir()

    def run_smoke_test(self):
        run_smoke_test_action(self)

    def stop_game(self):
        if self.running_proc:
            self.running_proc.terminate()
