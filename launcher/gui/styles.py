# SPDX-License-Identifier: GPL-2.0-or-later
"""TTK visual styling and dark-theme configurations for the launcher."""

from tkinter import ttk


def setup_theme_styles(root):
    """Configure modern dark palette styles across all TTK widgets."""
    style = ttk.Style(root)
    style.theme_use("clam")

    dark_bg = "#18181b"
    card_bg = "#27272a"
    text_fg = "#f4f4f5"
    accent = "#b91c1c"
    accent_hover = "#dc2626"

    style.configure(".", background=dark_bg, foreground=text_fg, font=("Segoe UI", 9))
    style.configure("TNotebook", background=dark_bg, borderwidth=0)
    style.configure(
        "TNotebook.Tab",
        background=card_bg,
        foreground=text_fg,
        padding=[12, 6],
        font=("Segoe UI", 9, "bold")
    )
    style.map(
        "TNotebook.Tab",
        background=[("selected", accent)],
        foreground=[("selected", "#ffffff")]
    )

    style.configure("TFrame", background=dark_bg)
    style.configure("Card.TFrame", background=card_bg, relief="flat")
    style.configure("TLabel", background=dark_bg, foreground=text_fg)
    style.configure("Card.TLabel", background=card_bg, foreground=text_fg)
    style.configure(
        "Header.TLabel",
        background=dark_bg,
        foreground="#ffffff",
        font=("Segoe UI", 16, "bold")
    )
    style.configure(
        "SubHeader.TLabel",
        background=dark_bg,
        foreground="#a1a1aa",
        font=("Segoe UI", 9)
    )

    style.configure(
        "Accent.TButton",
        background=accent,
        foreground="#ffffff",
        font=("Segoe UI", 10, "bold"),
        padding=[14, 6]
    )
    style.map("Accent.TButton", background=[("active", accent_hover), ("pressed", "#991b1b")])

    style.configure("Secondary.TButton", background="#3f3f46", foreground="#ffffff", padding=[10, 5])
    style.map("Secondary.TButton", background=[("active", "#52525b")])

    style.configure("TCheckbutton", background=card_bg, foreground=text_fg)
    style.map("TCheckbutton", background=[("active", card_bg)])

    style.configure("TCombobox", fieldbackground=card_bg, background="#3f3f46", foreground=text_fg)
    style.map("TCombobox", fieldbackground=[("readonly", card_bg)])
