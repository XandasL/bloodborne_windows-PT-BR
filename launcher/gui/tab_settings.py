# SPDX-License-Identifier: GPL-2.0-or-later
"""Settings and graphics options tab UI components."""

from tkinter import ttk


def build_settings_view(parent, app):
    """Build performance presets and graphical effects checkboxes."""
    card = ttk.Frame(parent, style="Card.TFrame", padding=12)
    card.pack(fill="both", expand=True, padx=4, pady=4)

    ttk.Label(card, text="PERFORMANCE & RESOLUTION", style="Card.TLabel",
              font=("Segoe UI", 10, "bold"), foreground="#ef4444").grid(row=0, column=0, columnspan=2, sticky="w", pady=(0, 8))

    ttk.Label(card, text="Target Framerate:", style="Card.TLabel").grid(row=1, column=0, sticky="w", pady=4)
    ttk.Combobox(card, textvariable=app.fps_var, values=["uncap", "60", "90", "30"],
                 state="readonly", width=18).grid(row=1, column=1, sticky="w", pady=4, padx=8)

    ttk.Label(card, text="Output Resolution:", style="Card.TLabel").grid(row=2, column=0, sticky="w", pady=4)
    ttk.Combobox(card, textvariable=app.resolution_var,
                 values=["1920x1080", "2560x1440", "3840x2160", "1280x720"],
                 state="readonly", width=18).grid(row=2, column=1, sticky="w", pady=4, padx=8)

    ttk.Label(card, text="Temporal Upscaler:", style="Card.TLabel").grid(row=3, column=0, sticky="w", pady=4)
    ttk.Combobox(card, textvariable=app.upscaler_var,
                 values=["fsr3", "fsr4", "fsr411", "taa", "off"],
                 state="readonly", width=18).grid(row=3, column=1, sticky="w", pady=4, padx=8)

    ttk.Label(card, text="Upscaler Quality Preset:", style="Card.TLabel").grid(row=4, column=0, sticky="w", pady=4)
    ttk.Combobox(card, textvariable=app.preset_var, values=["1", "2", "3", "0"],
                 state="readonly", width=18).grid(row=4, column=1, sticky="w", pady=4, padx=8)

    ttk.Label(card, text="GRAPHICAL EFFECTS & PATCHES", style="Card.TLabel",
              font=("Segoe UI", 10, "bold"), foreground="#ef4444").grid(row=5, column=0, columnspan=2, sticky="w", pady=(14, 8))

    fx_frame = ttk.Frame(card, style="Card.TFrame")
    fx_frame.grid(row=6, column=0, columnspan=2, sticky="w")

    ttk.Checkbutton(fx_frame, text="Skip Intro Videos", variable=app.skip_intro_var).grid(row=0, column=0, sticky="w", padx=6, pady=2)
    ttk.Checkbutton(fx_frame, text="Motion Blur", variable=app.motion_blur_var).grid(row=0, column=1, sticky="w", padx=6, pady=2)
    ttk.Checkbutton(fx_frame, text="Depth of Field (DoF)", variable=app.dof_var).grid(row=1, column=0, sticky="w", padx=6, pady=2)
    ttk.Checkbutton(fx_frame, text="Chromatic Aberration", variable=app.chromatic_aberration_var).grid(row=1, column=1, sticky="w", padx=6, pady=2)
    ttk.Checkbutton(fx_frame, text="Ambient Occlusion (SSAO)", variable=app.ssao_var).grid(row=2, column=0, sticky="w", padx=6, pady=2)
    ttk.Checkbutton(fx_frame, text="SSR Reflections (Experimental)", variable=app.ssr_var).grid(row=2, column=1, sticky="w", padx=6, pady=2)
    ttk.Checkbutton(fx_frame, text="Show In-Game FPS Overlay", variable=app.show_fps_var).grid(row=3, column=0, sticky="w", padx=6, pady=2)
