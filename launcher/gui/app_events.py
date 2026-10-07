# SPDX-License-Identifier: GPL-2.0-or-later
"""Event handlers and user interaction callbacks for launcher."""

import os
import subprocess
import sys
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
from bbport_i18n import tr
from .paths import MODS_DIR, MODS_CONFIG
from .sfo import verify_game_dump
from .mods_manager import list_installed_mods, get_disabled_mods, save_mods_state, extract_mod_archive


def on_browse_game_dir(app):
    p = filedialog.askdirectory(title=tr("Select Bloodborne Game Directory"))
    if p:
        app.game_dir_var.set(p)
        on_refresh_game_info(app)


def on_refresh_game_info(app):
    ok, msg = verify_game_dump(app.game_dir_var.get().strip())
    color = "#22c55e" if ok else "#ef4444"
    app.game_info_lbl.configure(text=msg, foreground=color)


def on_load_mods(app):
    for w in app.mod_inner_frame.winfo_children():
        w.destroy()
    dis = get_disabled_mods(MODS_CONFIG)
    app.mods_data = [
        (m, tk.BooleanVar(value=m not in dis)) for m in list_installed_mods(MODS_DIR)
    ]
    for name, var in app.mods_data:
        cb = ttk.Checkbutton(
            app.mod_inner_frame,
            text=name,
            variable=var,
            command=lambda: save_mods_state(MODS_CONFIG, app.mods_data)
        )
        cb.pack(anchor="w", padx=8, pady=3)


def on_add_mod(app):
    p = filedialog.askopenfilename(filetypes=[(tr("Mod Archives"), "*.zip")])
    if p:
        try:
            name = extract_mod_archive(p, MODS_DIR)
            messagebox.showinfo(tr("Success"), tr("Installed mod: {name}").format(name=name))
            on_load_mods(app)
        except Exception as e:
            messagebox.showerror(tr("Error"), str(e))


def on_open_mods_dir():
    if sys.platform == "win32":
        os.startfile(MODS_DIR)
    else:
        subprocess.Popen(["xdg-open", str(MODS_DIR)])
