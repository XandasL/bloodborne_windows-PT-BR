# SPDX-License-Identifier: GPL-2.0-or-later
"""Asynchronous actions and process dispatch for Bloodborne Launcher."""

import os
from pathlib import Path
import subprocess
import sys
import threading
from tkinter import messagebox
from bbport_i18n import tr
from .paths import BASE_DIR, CONFIG_FILE, PATCHES_DIR, find_probe_executable
from .ini_config import build_settings_payload, save_bbport_settings
from .runner import execute_preparation, spawn_game_process


def run_smoke_test_action(app):
    """Execute Vulkan hardware capability test asynchronously."""
    app.clear_log()
    app.log(tr("[SMOKE TEST] Initializing Vulkan hardware smoke test..."))
    probe = find_probe_executable()
    if not probe:
        messagebox.showerror(tr("Not Found"), tr("Could not find bb-probe executable."))
        return

    def worker():
        cmd = [str(probe), "--vulkan-only"]
        p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                             text=True, cwd=str(BASE_DIR))
        for line in p.stdout:
            app.log(line.rstrip())
        p.wait()
        app.log(tr("[SMOKE TEST] Completed with exit code {code}").format(code=p.returncode))

    threading.Thread(target=worker, daemon=True).start()


def launch_game_action(app):
    """Save settings, prepare assets in-process, and launch bb-probe."""
    gdir = app.game_dir_var.get().strip()
    if not gdir or not (Path(gdir) / "eboot.bin").is_file():
        messagebox.showerror(tr("Error"), tr("Please select a valid game folder containing 'eboot.bin'."))
        return

    save_bbport_settings(CONFIG_FILE, build_settings_payload(app))
    probe = find_probe_executable()
    if not probe:
        messagebox.showerror(tr("Not Found"), tr("Could not find bb-probe executable."))
        return

    app.launch_btn.configure(state="disabled")
    app.stop_btn.configure(state="normal")
    app.clear_log()
    app.log(tr("Preparing Bloodborne from: {path}").format(path=gdir))

    def worker():
        out_dir = BASE_DIR / "out"
        out_dir.mkdir(parents=True, exist_ok=True)
        try:
            execute_preparation(gdir, out_dir, app.fps_var.get(), CONFIG_FILE, PATCHES_DIR)
            app.log(tr(">> Booting Bloodborne via {probe}...").format(probe=probe.name))
            app.running_proc = spawn_game_process(probe, gdir, CONFIG_FILE, BASE_DIR)
            for line in app.running_proc.stdout:
                app.log(line.rstrip())
            app.running_proc.wait()
            app.log(tr(">> Game terminated (Exit code: {code})").format(code=app.running_proc.returncode))
        except Exception as e:
            app.log(tr("Error: {error}").format(error=e))
        finally:
            app.running_proc = None
            app.after(0, lambda: app.launch_btn.configure(state="normal"))
            app.after(0, lambda: app.stop_btn.configure(state="disabled"))

    threading.Thread(target=worker, daemon=True).start()
