# SPDX-License-Identifier: GPL-2.0-or-later
"""Mod Manager tab UI layout and scrollable mod list component."""

import tkinter as tk
from tkinter import ttk


def build_mods_view(parent, app):
    """Render the mod manager tab with add/refresh controls and mod list."""
    card = ttk.Frame(parent, style="Card.TFrame", padding=12)
    card.pack(fill="both", expand=True, padx=4, pady=4)

    top_bar = ttk.Frame(card, style="Card.TFrame")
    top_bar.pack(fill="x", pady=(0, 8))

    ttk.Label(top_bar, text="INSTALLED MODS", style="Card.TLabel",
              font=("Segoe UI", 10, "bold"), foreground="#ef4444").pack(side="left")

    ttk.Button(top_bar, text="➕ Add Mod (.zip)", style="Secondary.TButton",
               command=app.add_mod).pack(side="right", padx=(4, 0))
    ttk.Button(top_bar, text="📂 Open Folder", style="Secondary.TButton",
               command=app.open_mods_dir).pack(side="right", padx=(4, 0))
    ttk.Button(top_bar, text="🔄 Refresh", style="Secondary.TButton",
               command=app.load_mods).pack(side="right")

    list_frame = ttk.Frame(card, style="Card.TFrame")
    list_frame.pack(fill="both", expand=True)

    app.mod_canvas = tk.Canvas(list_frame, bg="#1c1c1f", highlightthickness=0)
    mod_scrollbar = ttk.Scrollbar(list_frame, orient="vertical", command=app.mod_canvas.yview)
    app.mod_inner_frame = ttk.Frame(app.mod_canvas, style="Card.TFrame")

    app.mod_inner_frame.bind(
        "<Configure>",
        lambda e: app.mod_canvas.configure(scrollregion=app.mod_canvas.bbox("all"))
    )
    app.mod_canvas.create_window((0, 0), window=app.mod_inner_frame, anchor="nw")
    app.mod_canvas.configure(yscrollcommand=mod_scrollbar.set)

    app.mod_canvas.pack(side="left", fill="both", expand=True)
    mod_scrollbar.pack(side="right", fill="y")
