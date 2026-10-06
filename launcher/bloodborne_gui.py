#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Bloodborne Cross-Platform Launcher & Mod Manager (GUI).

Standalone Windows/Linux graphical interface for configuring, managing mods,
and launching the Bloodborne HLE translation runner.
"""

import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import threading
import tkinter as tk
from tkinter import filedialog, messagebox, ttk
import zipfile

# Root repository directory
BASE_DIR = Path(__file__).resolve().parent.parent if Path(__file__).parent.name == "launcher" else Path.cwd()
SCRIPTS_DIR = BASE_DIR / "scripts"
MODS_DIR = BASE_DIR / "mods"
CONFIG_FILE = BASE_DIR / "bbport.ini"
MODS_CONFIG = BASE_DIR / "mods.json"

CUSA_CANDIDATES = [
    "CUSA00900", "CUSA03173", "CUSA00207", "CUSA00208",
    "CUSA03023", "CUSA01363", "CUSA00299", "CUSA03014"
]


def find_game_dir():
    """Auto-detect game folder in common relative locations."""
    env_dir = os.environ.get("BB_GAME_DIR")
    if env_dir and (Path(env_dir) / "eboot.bin").is_file():
        return str(Path(env_dir).resolve())

    for cusa in CUSA_CANDIDATES:
        for parent in [BASE_DIR, BASE_DIR.parent]:
            cand = parent / cusa
            if (cand / "eboot.bin").is_file():
                return str(cand.resolve())
    return ""


def read_sfo_metadata(sfo_path):
    """Extract TITLE_ID and APP_VER from param.sfo if present."""
    if not sfo_path.is_file():
        return {}
    try:
        data = sfo_path.read_bytes()
        if data[:4] != b'\x00PSF':
            return {}
        import struct
        key_off, val_off, count = struct.unpack('<III', data[8:20])
        entries = {}
        for i in range(count):
            k_off, param_fmt, param_len, param_max, v_off = struct.unpack('<HHIII', data[20 + i * 16:36 + i * 16])
            key = data[key_off + k_off:].split(b'\x00', 1)[0].decode('latin1')
            val = data[val_off + v_off:val_off + v_off + param_len].rstrip(b'\x00').decode('latin1', errors='ignore')
            entries[key] = val
        return entries
    except Exception:
        return {}


class BloodborneLauncherApp(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Bloodborne Runner Launcher & Mod Manager")
        self.geometry("820x680")
        self.minsize(740, 580)
        self.configure(bg="#18181b")

        self.game_dir_var = tk.StringVar(value=find_game_dir())
        self.fps_var = tk.StringVar(value="uncap")
        self.resolution_var = tk.StringVar(value="1920x1080")
        self.upscaler_var = tk.StringVar(value="fsr3")
        self.preset_var = tk.StringVar(value="1")  # Quality

        # Graphical effects
        self.skip_intro_var = tk.BooleanVar(value=True)
        self.motion_blur_var = tk.BooleanVar(value=True)
        self.dof_var = tk.BooleanVar(value=True)
        self.chromatic_aberration_var = tk.BooleanVar(value=True)
        self.ssao_var = tk.BooleanVar(value=True)
        self.ssr_var = tk.BooleanVar(value=False)
        self.show_fps_var = tk.BooleanVar(value=False)

        self.running_proc = None
        self.mods_data = []

        self.setup_styles()
        self.build_ui()
        self.refresh_game_info()
        self.load_mods()

    def setup_styles(self):
        style = ttk.Style(self)
        style.theme_use("clam")

        dark_bg = "#18181b"
        card_bg = "#27272a"
        text_fg = "#f4f4f5"
        accent = "#b91c1c"
        accent_hover = "#dc2626"

        style.configure(".", background=dark_bg, foreground=text_fg, font=("Segoe UI", 9))
        style.configure("TNotebook", background=dark_bg, borderwidth=0)
        style.configure("TNotebook.Tab", background=card_bg, foreground=text_fg, padding=[12, 6], font=("Segoe UI", 9, "bold"))
        style.map("TNotebook.Tab", background=[("selected", accent)], foreground=[("selected", "#ffffff")])

        style.configure("TFrame", background=dark_bg)
        style.configure("Card.TFrame", background=card_bg, relief="flat")
        style.configure("TLabel", background=dark_bg, foreground=text_fg)
        style.configure("Card.TLabel", background=card_bg, foreground=text_fg)
        style.configure("Header.TLabel", background=dark_bg, foreground="#ffffff", font=("Segoe UI", 16, "bold"))
        style.configure("SubHeader.TLabel", background=dark_bg, foreground="#a1a1aa", font=("Segoe UI", 9))

        style.configure("Accent.TButton", background=accent, foreground="#ffffff", font=("Segoe UI", 10, "bold"), padding=[14, 6])
        style.map("Accent.TButton", background=[("active", accent_hover), ("pressed", "#991b1b")])

        style.configure("Secondary.TButton", background="#3f3f46", foreground="#ffffff", padding=[10, 5])
        style.map("Secondary.TButton", background=[("active", "#52525b")])

        style.configure("TCheckbutton", background=card_bg, foreground=text_fg)
        style.map("TCheckbutton", background=[("active", card_bg)])

        style.configure("TCombobox", fieldbackground=card_bg, background="#3f3f46", foreground=text_fg, selectbackground=accent)
        style.map("TCombobox", fieldbackground=[("readonly", card_bg)], selectbackground=[("readonly", accent)])

    def build_ui(self):
        # 1. Header banner
        header_frame = ttk.Frame(self)
        header_frame.pack(fill="x", padx=16, pady=(12, 6))

        title_lbl = ttk.Label(header_frame, text="BLOODBORNE RUNNER", style="Header.TLabel")
        title_lbl.pack(anchor="w")
        sub_lbl = ttk.Label(header_frame, text="Cross-Platform HLE Recompiler & Translation Launcher", style="SubHeader.TLabel")
        sub_lbl.pack(anchor="w")

        # 2. Game directory selection card
        dir_card = ttk.Frame(self, style="Card.TFrame", padding=10)
        dir_card.pack(fill="x", padx=16, pady=6)

        dir_lbl = ttk.Label(dir_card, text="Game Directory (CUSA00900 / CUSA03173):", style="Card.TLabel", font=("Segoe UI", 9, "bold"))
        dir_lbl.grid(row=0, column=0, sticky="w", pady=(0, 4))

        dir_entry = ttk.Entry(dir_card, textvariable=self.game_dir_var, font=("Segoe UI", 9))
        dir_entry.grid(row=1, column=0, sticky="ew", padx=(0, 8))
        dir_card.columnconfigure(0, weight=1)

        browse_btn = ttk.Button(dir_card, text="Browse...", style="Secondary.TButton", command=self.browse_game_dir)
        browse_btn.grid(row=1, column=1)

        self.game_info_lbl = ttk.Label(dir_card, text="Checking game files...", style="Card.TLabel", foreground="#e4e4e7")
        self.game_info_lbl.grid(row=2, column=0, columnspan=2, sticky="w", pady=(6, 0))

        # 3. Notebook / Tabs
        notebook = ttk.Notebook(self)
        notebook.pack(fill="both", expand=True, padx=16, pady=6)

        tab_settings = ttk.Frame(notebook)
        tab_mods = ttk.Frame(notebook)
        tab_diag = ttk.Frame(notebook)

        notebook.add(tab_settings, text=" Settings & Graphics ")
        notebook.add(tab_mods, text=" Mod Manager ")
        notebook.add(tab_diag, text=" Diagnostics ")

        self.build_settings_tab(tab_settings)
        self.build_mods_tab(tab_mods)
        self.build_diag_tab(tab_diag)

        # 4. Action bar & launch button
        action_bar = ttk.Frame(self)
        action_bar.pack(fill="x", padx=16, pady=8)

        self.launch_btn = ttk.Button(action_bar, text="▶  LAUNCH BLOODBORNE", style="Accent.TButton", command=self.launch_game)
        self.launch_btn.pack(side="left", padx=(0, 8))

        self.stop_btn = ttk.Button(action_bar, text="⏹ Stop", style="Secondary.TButton", command=self.stop_game, state="disabled")
        self.stop_btn.pack(side="left", padx=(0, 8))

        smoke_btn = ttk.Button(action_bar, text="🔍 Vulkan Smoke Test", style="Secondary.TButton", command=self.run_smoke_test)
        smoke_btn.pack(side="left")

        # 5. Output Console Drawer
        self.build_console_drawer()

    def build_settings_tab(self, parent):
        card = ttk.Frame(parent, style="Card.TFrame", padding=12)
        card.pack(fill="both", expand=True, padx=4, pady=4)

        # Performance Grid
        ttk.Label(card, text="PERFORMANCE & RESOLUTION", style="Card.TLabel", font=("Segoe UI", 10, "bold"), foreground="#ef4444").grid(row=0, column=0, columnspan=2, sticky="w", pady=(0, 8))

        ttk.Label(card, text="Target Framerate:", style="Card.TLabel").grid(row=1, column=0, sticky="w", pady=4)
        fps_combo = ttk.Combobox(card, textvariable=self.fps_var, values=["uncap", "60", "90", "30"], state="readonly", width=18)
        fps_combo.grid(row=1, column=1, sticky="w", pady=4, padx=8)

        ttk.Label(card, text="Output Resolution:", style="Card.TLabel").grid(row=2, column=0, sticky="w", pady=4)
        res_combo = ttk.Combobox(card, textvariable=self.resolution_var, values=["1920x1080", "2560x1440", "3840x2160", "1280x720"], state="readonly", width=18)
        res_combo.grid(row=2, column=1, sticky="w", pady=4, padx=8)

        ttk.Label(card, text="Temporal Upscaler:", style="Card.TLabel").grid(row=3, column=0, sticky="w", pady=4)
        up_combo = ttk.Combobox(card, textvariable=self.upscaler_var, values=["fsr3", "fsr4", "fsr411", "taa", "off"], state="readonly", width=18)
        up_combo.grid(row=3, column=1, sticky="w", pady=4, padx=8)

        ttk.Label(card, text="Upscaler Quality Preset:", style="Card.TLabel").grid(row=4, column=0, sticky="w", pady=4)
        preset_combo = ttk.Combobox(card, textvariable=self.preset_var, values=["1", "2", "3", "0"], state="readonly", width=18)
        preset_combo.grid(row=4, column=1, sticky="w", pady=4, padx=8)

        # Effects Grid
        ttk.Label(card, text="GRAPHICAL EFFECTS & PATCHES", style="Card.TLabel", font=("Segoe UI", 10, "bold"), foreground="#ef4444").grid(row=5, column=0, columnspan=2, sticky="w", pady=(14, 8))

        fx_frame = ttk.Frame(card, style="Card.TFrame")
        fx_frame.grid(row=6, column=0, columnspan=2, sticky="w")

        ttk.Checkbutton(fx_frame, text="Skip Intro Videos", variable=self.skip_intro_var).grid(row=0, column=0, sticky="w", padx=6, pady=2)
        ttk.Checkbutton(fx_frame, text="Motion Blur", variable=self.motion_blur_var).grid(row=0, column=1, sticky="w", padx=6, pady=2)
        ttk.Checkbutton(fx_frame, text="Depth of Field (DoF)", variable=self.dof_var).grid(row=1, column=0, sticky="w", padx=6, pady=2)
        ttk.Checkbutton(fx_frame, text="Chromatic Aberration", variable=self.chromatic_aberration_var).grid(row=1, column=1, sticky="w", padx=6, pady=2)
        ttk.Checkbutton(fx_frame, text="Ambient Occlusion (SSAO)", variable=self.ssao_var).grid(row=2, column=0, sticky="w", padx=6, pady=2)
        ttk.Checkbutton(fx_frame, text="SSR Reflections (Experimental)", variable=self.ssr_var).grid(row=2, column=1, sticky="w", padx=6, pady=2)
        ttk.Checkbutton(fx_frame, text="Show In-Game FPS Overlay", variable=self.show_fps_var).grid(row=3, column=0, sticky="w", padx=6, pady=2)

    def build_mods_tab(self, parent):
        card = ttk.Frame(parent, style="Card.TFrame", padding=12)
        card.pack(fill="both", expand=True, padx=4, pady=4)

        top_bar = ttk.Frame(card, style="Card.TFrame")
        top_bar.pack(fill="x", pady=(0, 8))

        ttk.Label(top_bar, text="INSTALLED MODS", style="Card.TLabel", font=("Segoe UI", 10, "bold"), foreground="#ef4444").pack(side="left")

        add_mod_btn = ttk.Button(top_bar, text="➕ Add Mod (.zip / folder)", style="Secondary.TButton", command=self.add_mod)
        add_mod_btn.pack(side="right", padx=(4, 0))

        open_folder_btn = ttk.Button(top_bar, text="📂 Open Mods Folder", style="Secondary.TButton", command=self.open_mods_dir)
        open_folder_btn.pack(side="right", padx=(4, 0))

        refresh_btn = ttk.Button(top_bar, text="🔄 Refresh", style="Secondary.TButton", command=self.load_mods)
        refresh_btn.pack(side="right")

        # Mod list frame with canvas for scrolling
        list_frame = ttk.Frame(card, style="Card.TFrame")
        list_frame.pack(fill="both", expand=True)

        self.mod_canvas = tk.Canvas(list_frame, bg="#1c1c1f", highlightthickness=0)
        mod_scrollbar = ttk.Scrollbar(list_frame, orient="vertical", command=self.mod_canvas.yview)
        self.mod_inner_frame = ttk.Frame(self.mod_canvas, style="Card.TFrame")

        self.mod_inner_frame.bind("<Configure>", lambda e: self.mod_canvas.configure(scrollregion=self.mod_canvas.bbox("all")))
        self.mod_canvas.create_window((0, 0), window=self.mod_inner_frame, anchor="nw")
        self.mod_canvas.configure(xscrollcommand=None, yscrollcommand=mod_scrollbar.set)

        self.mod_canvas.pack(side="left", fill="both", expand=True)
        mod_scrollbar.pack(side="right", fill="y")

    def build_diag_tab(self, parent):
        card = ttk.Frame(parent, style="Card.TFrame", padding=12)
        card.pack(fill="both", expand=True, padx=4, pady=4)

        ttk.Label(card, text="HARDWARE & RUNTIME DIAGNOSTICS", style="Card.TLabel", font=("Segoe UI", 10, "bold"), foreground="#ef4444").pack(anchor="w", pady=(0, 8))

        diag_text = (
            "• OS: Windows 64-bit / Linux x86-64\n"
            "• Architecture: Zero CPU Emulation (SysV AMD64 ABI Direct Call Gates)\n"
            "• Graphics: Vulkan 1.3 translation layer with temporal reconstruction\n"
            "• Memory: Direct Win32 Pagefile Section physical memory aliasing (< 1 TiB)\n\n"
            "To verify your graphics drivers and swapchain negotiation without running the game,\n"
            "click 'Vulkan Smoke Test' below or in the action bar."
        )
        ttk.Label(card, text=diag_text, style="Card.TLabel", justify="left").pack(anchor="w", pady=4)

        test_btn = ttk.Button(card, text="Run Vulkan Smoke Test Now", style="Secondary.TButton", command=self.run_smoke_test)
        test_btn.pack(anchor="w", pady=10)

    def build_console_drawer(self):
        console_frame = ttk.Frame(self)
        console_frame.pack(fill="x", padx=16, pady=(0, 10))

        lbl_frame = ttk.Frame(console_frame)
        lbl_frame.pack(fill="x")
        ttk.Label(lbl_frame, text="Activity Log:", font=("Segoe UI", 8, "bold"), foreground="#a1a1aa").pack(side="left")

        clear_btn = ttk.Button(lbl_frame, text="Clear", style="Secondary.TButton", command=self.clear_log)
        clear_btn.pack(side="right")

        self.log_text = tk.Text(console_frame, height=7, bg="#111113", fg="#d4d4d8", insertbackground="#ffffff", font=("Consolas", 8), relief="flat")
        self.log_text.pack(fill="x", pady=(2, 0))

    def log(self, text):
        self.log_text.insert("end", text + "\n")
        self.log_text.see("end")

    def clear_log(self):
        self.log_text.delete("1.0", "end")

    def browse_game_dir(self):
        chosen = filedialog.askdirectory(title="Select Bloodborne Game Directory (CUSA00900 / CUSA03173)")
        if chosen:
            self.game_dir_var.set(chosen)
            self.refresh_game_info()

    def refresh_game_info(self):
        gdir = Path(self.game_dir_var.get().strip())
        if not gdir.is_dir():
            self.game_info_lbl.configure(text="❌ Directory does not exist. Click 'Browse...' to select your dumped game.", foreground="#ef4444")
            return

        eboot = gdir / "eboot.bin"
        if not eboot.is_file():
            self.game_info_lbl.configure(text="⚠️ Directory found, but 'eboot.bin' is missing inside.", foreground="#f59e0b")
            return

        # Check SELF header
        try:
            head = eboot.read_bytes()[:4]
            if head != b'O\x15=\x1d':
                self.game_info_lbl.configure(text="⚠️ 'eboot.bin' does not have a standard PS4 SELF header.", foreground="#f59e0b")
                return
        except Exception as e:
            self.game_info_lbl.configure(text=f"⚠️ Cannot read eboot.bin: {e}", foreground="#f59e0b")
            return

        meta = read_sfo_metadata(gdir / "sce_sys" / "param.sfo")
        title_id = meta.get("TITLE_ID", gdir.name)
        app_ver = meta.get("APP_VER", "Unknown")

        self.game_info_lbl.configure(
            text=f"✅ Ready: Title ID: {title_id} | Version: {app_ver} | Verified plaintext ELF",
            foreground="#22c55e"
        )

    def load_mods(self):
        for widget in self.mod_inner_frame.winfo_children():
            widget.destroy()

        MODS_DIR.mkdir(parents=True, exist_ok=True)
        disabled_mods = set()
        if MODS_CONFIG.is_file():
            try:
                cfg = json.loads(MODS_CONFIG.read_text())
                disabled_mods = set(cfg.get("disabled", []))
            except Exception:
                pass

        mods = sorted([p.name for p in MODS_DIR.iterdir() if p.is_dir()])
        self.mods_data = []

        if not mods:
            ttk.Label(self.mod_inner_frame, text="No mods installed. Click 'Add Mod' to install mod packages.", style="Card.TLabel", foreground="#71717a").pack(anchor="w", padx=8, pady=8)
            return

        for name in mods:
            var = tk.BooleanVar(value=(name not in disabled_mods))
            cb = ttk.Checkbutton(self.mod_inner_frame, text=name, variable=var, command=self.save_mods_config)
            cb.pack(anchor="w", padx=8, pady=4)
            self.mods_data.append((name, var))

    def save_mods_config(self):
        disabled = [name for name, var in self.mods_data if not var.get()]
        cfg = {"disabled": disabled, "order": [name for name, _ in self.mods_data]}
        try:
            MODS_CONFIG.write_text(json.dumps(cfg, indent=2))
            self.log(f"Mods config updated: {len(self.mods_data) - len(disabled)} active, {len(disabled)} disabled.")
        except Exception as e:
            self.log(f"Failed to save mods config: {e}")

    def add_mod(self):
        path = filedialog.askopenfilename(
            title="Select Mod Package (.zip) or select directory",
            filetypes=[("Mod Archives", "*.zip"), ("All Files", "*.*")]
        )
        if not path:
            return

        src = Path(path)
        if src.suffix.lower() == ".zip":
            mod_name = src.stem
            target_dir = MODS_DIR / mod_name
            try:
                with zipfile.ZipFile(src, "r") as zf:
                    zf.extractall(target_dir)
                messagebox.showinfo("Mod Added", f"Mod '{mod_name}' extracted successfully!")
                self.load_mods()
            except Exception as e:
                messagebox.showerror("Error", f"Failed to extract mod: {e}")

    def open_mods_dir(self):
        MODS_DIR.mkdir(parents=True, exist_ok=True)
        if sys.platform == "win32":
            os.startfile(MODS_DIR)
        else:
            subprocess.Popen(["xdg-open", str(MODS_DIR)])

    def save_bbport_ini(self):
        """Persist UI options to bbport.ini."""
        lines = []
        if CONFIG_FILE.is_file():
            lines = CONFIG_FILE.read_text().splitlines()

        settings = {
            "upscaler": self.upscaler_var.get(),
            "preset": self.preset_var.get(),
            "output_res": self.resolution_var.get(),
            "show_fps": "1" if self.show_fps_var.get() else "0",
            "skip_intro": "1" if self.skip_intro_var.get() else "0",
            "effect_motion_blur": "1" if self.motion_blur_var.get() else "0",
            "effect_dof": "1" if self.dof_var.get() else "0",
            "effect_chromatic_aberration": "1" if self.chromatic_aberration_var.get() else "0",
            "effect_ssao": "1" if self.ssao_var.get() else "0",
            "effect_ssr": "1" if self.ssr_var.get() else "0"
        }

        # Update existing keys or append
        existing_keys = set()
        new_lines = []
        for line in lines:
            if "=" in line and not line.strip().startswith(("#", ";")):
                k, v = line.split("=", 1)
                k = k.strip()
                if k in settings:
                    new_lines.append(f"{k} = {settings[k]}")
                    existing_keys.add(k)
                    continue
            new_lines.append(line)

        for k, v in settings.items():
            if k not in existing_keys:
                new_lines.append(f"{k} = {v}")

        CONFIG_FILE.write_text("\n".join(new_lines) + "\n")

    def run_smoke_test(self):
        self.clear_log()
        self.log("[SMOKE TEST] Initializing Vulkan hardware smoke test...")
        probe_exe = self.find_probe_executable()
        if not probe_exe:
            messagebox.showerror("Not Found", "Could not find bb-probe executable. Please compile or download the release.")
            return

        def worker():
            cmd = [str(probe_exe), "--vulkan-only"]
            proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, cwd=str(BASE_DIR))
            for line in proc.stdout:
                self.log(line.rstrip())
            proc.wait()
            self.log(f"[SMOKE TEST] Completed with exit code {proc.returncode}")

        threading.Thread(target=worker, daemon=True).start()

    def find_probe_executable(self):
        for cand in [
            BASE_DIR / "bb-probe.exe",
            BASE_DIR / "bb-probe",
            BASE_DIR / "out" / "bb-probe.exe",
            BASE_DIR / "build" / "bb-probe.exe",
            BASE_DIR / "build" / "bb-probe"
        ]:
            if cand.is_file():
                return cand.resolve()
        return None

    def launch_game(self):
        gdir = self.game_dir_var.get().strip()
        if not gdir or not (Path(gdir) / "eboot.bin").is_file():
            messagebox.showerror("Error", "Please select a valid game folder containing 'eboot.bin'.")
            return

        self.save_bbport_ini()
        probe_exe = self.find_probe_executable()
        if not probe_exe:
            messagebox.showerror("Not Found", "Could not find bb-probe executable.")
            return

        self.launch_btn.configure(state="disabled")
        self.stop_btn.configure(state="normal")
        self.clear_log()
        self.log(f"Launching Bloodborne from: {gdir}")

        def worker():
            py_exe = sys.executable
            out_dir = BASE_DIR / "out"
            out_dir.mkdir(parents=True, exist_ok=True)

            steps = [
                ("Preparing binary image", [py_exe, str(SCRIPTS_DIR / "prepare.py"), gdir, "--out", str(out_dir)]),
                ("Linking libc exports", [py_exe, str(SCRIPTS_DIR / "link_libc.py"), gdir, "--out", str(out_dir)]),
                ("Linking guest modules", [py_exe, str(SCRIPTS_DIR / "link_modules.py"), gdir, "--out", str(out_dir)]),
                ("Applying content profile & patches", [py_exe, str(SCRIPTS_DIR / "patches.py"), "--out", str(out_dir), "--fps", self.fps_var.get(), "--game-dir", gdir])
            ]

            for desc, cmd in steps:
                self.log(f">> {desc}...")
                p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True, cwd=str(BASE_DIR))
                for line in p.stdout:
                    self.log(f"   {line.rstrip()}")
                p.wait()
                if p.returncode != 0:
                    self.log(f"❌ Failed: {desc} exited with code {p.returncode}")
                    self.launch_btn.configure(state="normal")
                    self.stop_btn.configure(state="disabled")
                    return

            self.log(">> Booting Bloodborne via bb-probe...")
            env = os.environ.copy()
            env["BB_GAME_DIR"] = gdir

            self.running_proc = subprocess.Popen(
                [str(probe_exe)],
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
                cwd=str(BASE_DIR),
                env=env
            )

            for line in self.running_proc.stdout:
                self.log(line.rstrip())

            self.running_proc.wait()
            self.log(f">> Game process terminated (Code: {self.running_proc.returncode})")
            self.running_proc = None
            self.launch_btn.configure(state="normal")
            self.stop_btn.configure(state="disabled")

        threading.Thread(target=worker, daemon=True).start()

    def stop_game(self):
        if self.running_proc:
            self.log("Stopping game process...")
            self.running_proc.terminate()


if __name__ == "__main__":
    app = BloodborneLauncherApp()
    app.mainloop()
