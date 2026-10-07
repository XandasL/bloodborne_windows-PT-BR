# SPDX-License-Identifier: GPL-2.0-or-later
"""Performance and developer settings sections."""

from bbport_i18n import tr
from gi.repository import Adw
from launcher.gtk.constants import DRAW_PIPE, READBACKS
from launcher.gtk.widgets import combo_row


def build_perf_and_dev_groups(win):
    perf = Adw.PreferencesGroup(title=tr("Performance"))
    win.pipe_row = combo_row(
        tr("Two-stage GPU pipeline"), tr("20–30% faster; turn off if unstable"),
        DRAW_PIPE, win.settings["draw_pipe"]
    )
    perf.add(win.pipe_row)
    win.readbacks_row = combo_row(
        tr("GPU data readbacks by the CPU"), None, READBACKS, win.settings["readbacks"]
    )
    perf.add(win.readbacks_row)

    dev = Adw.PreferencesGroup(title=tr("Developer"))
    win.mangohud_row = Adw.SwitchRow(title="MangoHud", active=win.settings["mangohud"])
    dev.add(win.mangohud_row)
    win.stats_row = Adw.SwitchRow(
        title=tr("Frame statistics in the log"), subtitle="BB_FRAME_STATS",
        active=win.settings["frame_stats"]
    )
    dev.add(win.stats_row)
    win.profile_row = Adw.SwitchRow(
        title=tr("GPU profile in the log"), subtitle="BB_GPU_PROFILE",
        active=win.settings["gpu_profile"]
    )
    dev.add(win.profile_row)
    win.validation_row = Adw.SwitchRow(
        title=tr("Vulkan validation layers"), subtitle=tr("Much slower"),
        active=win.settings["vk_validation"]
    )
    dev.add(win.validation_row)
    win.extra_row = Adw.EntryRow(
        title=tr("Extra variables (NAME=value, space-separated)"),
        text=win.settings["extra_env"]
    )
    dev.add(win.extra_row)
    return perf, dev
