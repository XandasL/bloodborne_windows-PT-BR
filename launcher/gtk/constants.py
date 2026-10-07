# SPDX-License-Identifier: GPL-2.0-or-later
"""Constants, options, and defaults for GTK launcher."""

import os
from pathlib import Path

PORT_DIR = Path(__file__).resolve().parent.parent.parent
PACKAGED = bool(os.environ.get("BB_PREBUILT"))
DATA_DIR = Path(os.environ.get("BB_DATA_DIR", PORT_DIR))
CONFIG_DIR = Path(os.environ.get("XDG_CONFIG_HOME", Path.home() / ".config")) / "bbport-launcher"
CONFIG_FILE = CONFIG_DIR / "settings.json"
MAX_LOG_LINES = 5000

UI_LANGUAGES = [("System", ""), ("Russian", "ru"), ("English", "en")]
UPSCALERS = [("FSR 4", "fsr4"), ("FSR 4.1.1", "fsr411"), ("FSR 3", "fsr3"),
             ("TAA (native anti-aliasing)", "taa"), ("Off", "off")]
PRESETS = [("Native AA", 0), ("Quality (x1.5)", 1), ("Balanced (x1.7)", 2),
           ("Performance (x2)", 3), ("Ultra Performance (x3)", 4)]
OUTPUT_RES = [("1280×720 (Steam Deck)", "1280x720"), ("1920×1080", "1920x1080"),
              ("2560×1440", "2560x1440"), ("3840×2160", "3840x2160")]

EFFECTS = [
    ("effect_chromatic_aberration", "Chromatic aberration", True),
    ("effect_dof", "Depth of field (DoF)", True),
    ("effect_motion_blur", "Motion blur", True),
    ("effect_ssao", "SSAO", True),
    ("effect_game_aa", "The game's own anti-aliasing", True),
    ("effect_dynamic_shadows", "Dynamic light shadows", True),
    ("effect_ssr", "SSR reflections (not in the original game)", False),
    ("skip_intro", "Skip the intro videos", False),
    ("debug_camera", "Free camera (Cross + L3 / Space + Z)", False),
    ("debug_menu", "Debug menu (left touchpad / Tab; needs fonts)", False),
]

MODEL_LOD = [("As in the game", "0"), ("Highest (-2)", "-2"), ("Lower (1)", "1"), ("Lowest (2)", "2")]
FPS_MODES = [("Unlocked (patch)", "uncap"), ("60", "60"), ("90", "90"), ("30 (as on PS4)", "30")]
PRESENT_MODES = [("Mailbox", "Mailbox"), ("FIFO (VSync)", "Fifo"),
                 ("FIFO Relaxed", "FifoRelaxed"), ("Immediate", "Immediate")]
DRAW_PIPE = [("Auto (8+ threads)", ""), ("On", "1"), ("Off (more stable)", "0")]
LANGUAGES = [("English", "1"), ("Russian", "8"), ("Japanese", "0"), ("French", "2"),
             ("Spanish", "3"), ("German", "4"), ("Italian", "5")]
LIVE_RESOLUTION = [("Auto (by GPU)", "auto"), ("Off (faster)", "0"), ("On", "1")]
READBACKS = [("Relaxed (default)", ""), ("Off", "0"), ("Precise", "2")]

DEFAULTS = {
    "ui_language": "",
    "game_dir": "" if PACKAGED else str(PORT_DIR.parent / "CUSA03173"),
    "user_dir": "", "mods_dir": "", "mods_enabled": True, "patches_dir": "",
    "language": "1", "fullscreen": False, "hdr": False, "present_mode": "Mailbox",
    "fps_mode": "uncap", "fps_limit": 0, "draw_pipe": "", "readbacks": "",
    "mangohud": False, "frame_stats": False, "gpu_profile": False,
    "vk_validation": False, "extra_env": "",
}

INI_DEFAULTS = {
    "upscaler": "fsr4", "preset": "4", "sharpen": "1", "sharpness": "0.50",
    "object_motion": "1", "show_fps": "1", "output_res": "1920x1080",
    "model_lod": "0", "live_resolution": "0",
    **{key: "1" if default else "0" for key, _, default in EFFECTS},
}
