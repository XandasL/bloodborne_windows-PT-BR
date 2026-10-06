# SPDX-License-Identifier: GPL-2.0-or-later
"""Hardware diagnostics and Vulkan verification tab layout."""

from tkinter import ttk


def build_diag_view(parent, app):
    """Render the diagnostics description and Vulkan smoke test launcher."""
    card = ttk.Frame(parent, style="Card.TFrame", padding=12)
    card.pack(fill="both", expand=True, padx=4, pady=4)

    ttk.Label(
        card,
        text="HARDWARE & RUNTIME DIAGNOSTICS",
        style="Card.TLabel",
        font=("Segoe UI", 10, "bold"),
        foreground="#ef4444"
    ).pack(anchor="w", pady=(0, 8))

    diag_text = (
        "• OS: Windows 64-bit / Linux x86-64\n"
        "• Architecture: Zero CPU Emulation (SysV AMD64 ABI Direct Call Gates)\n"
        "• Graphics: Vulkan 1.3 translation layer with temporal reconstruction\n"
        "• Memory: Direct Win32 Pagefile Section physical memory aliasing (< 1 TiB)\n\n"
        "To verify your graphics drivers and swapchain negotiation without running the game,\n"
        "click 'Run Vulkan Smoke Test Now' below."
    )
    ttk.Label(card, text=diag_text, style="Card.TLabel", justify="left").pack(anchor="w", pady=4)

    ttk.Button(
        card,
        text="Run Vulkan Smoke Test Now",
        style="Secondary.TButton",
        command=app.run_smoke_test
    ).pack(anchor="w", pady=10)
