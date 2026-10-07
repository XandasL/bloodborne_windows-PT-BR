# SPDX-License-Identifier: GPL-2.0-or-later
"""Display and upscaler settings sections."""

from bbport_i18n import tr
from gi.repository import Adw
from launcher.gtk.constants import (
    LIVE_RESOLUTION, OUTPUT_RES, PRESENT_MODES, PRESETS, UPSCALERS
)
from launcher.gtk.widgets import combo_row


def build_display_group(win):
    screen = Adw.PreferencesGroup(title=tr("Display"))
    win.output_row = combo_row(
        tr("Output resolution"),
        tr("The upscaler fills the frame; Steam Deck: 720p"),
        OUTPUT_RES, win.ini.get("output_res", "1920x1080")
    )
    screen.add(win.output_row)
    win.live_row = combo_row(
        tr("Live resolution changes"),
        tr("No restart needed, but slower on the Steam Deck and older GPUs"),
        LIVE_RESOLUTION, win.ini.get("live_resolution", "0")
    )
    screen.add(win.live_row)
    win.fullscreen_row = Adw.SwitchRow(
        title=tr("Fullscreen"), active=win.settings["fullscreen"]
    )
    screen.add(win.fullscreen_row)
    win.present_row = combo_row(
        tr("Present mode"), None, PRESENT_MODES, win.settings["present_mode"]
    )
    screen.add(win.present_row)
    win.hdr_row = Adw.SwitchRow(title=tr("Allow HDR"), active=win.settings["hdr"])
    screen.add(win.hdr_row)
    return screen


def build_upscaler_group(win):
    upscaler = Adw.PreferencesGroup(
        title=tr("Upscaler"),
        description=tr("Stored in bbport.ini; in game, change it in the menu (Insert or L3+R3)")
    )
    win.upscaler_row = combo_row(tr("Upscaler"), None, UPSCALERS, win.ini["upscaler"])
    win.upscaler_row.connect("notify::selected", lambda *_: win.update_upscaler_status())
    upscaler.add(win.upscaler_row)
    win.preset_row = combo_row(
        tr("Preset"), None, PRESETS, int(win.ini.get("preset", "4"))
    )
    win.preset_row.connect("notify::selected", lambda *_: win.update_upscaler_status())
    win.output_row.connect("notify::selected", lambda *_: win.update_upscaler_status())
    upscaler.add(win.preset_row)
    win.sharpen_row = Adw.SwitchRow(
        title=tr("Sharpening (RCAS)"), active=win.ini.get("sharpen") == "1"
    )
    upscaler.add(win.sharpen_row)
    win.sharpness_row = Adw.SpinRow.new_with_range(0.0, 2.0, 0.05)
    win.sharpness_row.set_title(tr("Sharpness"))
    win.sharpness_row.set_digits(2)
    win.sharpness_row.set_value(float(win.ini.get("sharpness", "0.5")))
    upscaler.add(win.sharpness_row)
    win.motion_row = Adw.SwitchRow(
        title=tr("Object motion vectors"),
        subtitle=tr("Less ghosting on characters; costs about 10% FPS"),
        active=win.ini.get("object_motion") == "1"
    )
    upscaler.add(win.motion_row)
    win.show_fps_row = Adw.SwitchRow(
        title=tr("Show FPS"), active=win.ini.get("show_fps") == "1"
    )
    upscaler.add(win.show_fps_row)
    return upscaler
