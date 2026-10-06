# SPDX-License-Identifier: GPL-2.0-or-later
"""Layout assembly for Bloodborne Launcher window."""

import tkinter as tk
from tkinter import ttk
from .tab_settings import build_settings_view
from .tab_mods import build_mods_view
from .tab_diag import build_diag_view
from .actions import run_smoke_test_action, launch_game_action
from .app_events import on_browse_game_dir


def assemble_ui_components(app):
    """Build all frames, tabs, and action buttons on the app window."""
    hdr = ttk.Frame(app)
    hdr.pack(fill="x", padx=16, pady=(12, 6))
    ttk.Label(hdr, text="BLOODBORNE RUNNER", style="Header.TLabel").pack(anchor="w")
    ttk.Label(hdr, text="Cross-Platform HLE Recompiler & Translation Launcher", style="SubHeader.TLabel").pack(anchor="w")

    dir_card = ttk.Frame(app, style="Card.TFrame", padding=10)
    dir_card.pack(fill="x", padx=16, pady=6)
    ttk.Label(dir_card, text="Game Directory:", style="Card.TLabel", font=("Segoe UI", 9, "bold")).grid(row=0, column=0, sticky="w")
    ttk.Entry(dir_card, textvariable=app.game_dir_var).grid(row=1, column=0, sticky="ew", padx=(0, 8))
    dir_card.columnconfigure(0, weight=1)
    ttk.Button(dir_card, text="Browse...", style="Secondary.TButton", command=lambda: on_browse_game_dir(app)).grid(row=1, column=1)
    app.game_info_lbl = ttk.Label(dir_card, text="", style="Card.TLabel")
    app.game_info_lbl.grid(row=2, column=0, columnspan=2, sticky="w", pady=(4, 0))

    nb = ttk.Notebook(app)
    nb.pack(fill="both", expand=True, padx=16, pady=6)
    t_set, t_mod, t_diag = ttk.Frame(nb), ttk.Frame(nb), ttk.Frame(nb)
    nb.add(t_set, text=" Settings & Graphics ")
    nb.add(t_mod, text=" Mod Manager ")
    nb.add(t_diag, text=" Diagnostics ")
    build_settings_view(t_set, app)
    build_mods_view(t_mod, app)
    build_diag_view(t_diag, app)

    bar = ttk.Frame(app)
    bar.pack(fill="x", padx=16, pady=8)
    app.launch_btn = ttk.Button(bar, text="▶  LAUNCH BLOODBORNE", style="Accent.TButton", command=lambda: launch_game_action(app))
    app.launch_btn.pack(side="left", padx=(0, 8))
    app.stop_btn = ttk.Button(bar, text="⏹ Stop", style="Secondary.TButton", command=app.stop_game, state="disabled")
    app.stop_btn.pack(side="left", padx=(0, 8))
    ttk.Button(bar, text="🔍 Vulkan Smoke Test", style="Secondary.TButton", command=lambda: run_smoke_test_action(app)).pack(side="left")

    log_box = ttk.Frame(app)
    log_box.pack(fill="x", padx=16, pady=(0, 10))
    app.log_text = tk.Text(log_box, height=6, bg="#111113", fg="#d4d4d8", font=("Consolas", 8), relief="flat")
    app.log_text.pack(fill="x")
