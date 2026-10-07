# SPDX-License-Identifier: GPL-2.0-or-later
"""Mods and third-party patches settings section."""

from bbport_i18n import tr
from gi.repository import Adw
from launcher.gtk.config import patches_dir
from launcher.gtk.widgets import flat_button, open_folder, FolderList


def build_mods_group(win):
    desc = tr(
        "Extract each mod into its own folder (with dvdroot_ps4, or chr/, parts/, ... directly). "
        "When files collide, the mod lower in the list wins. Applied at start."
    )
    win.mods_group = Adw.PreferencesGroup(title=tr("Mods"), description=desc)
    win.mods_enabled_row = Adw.SwitchRow(
        title=tr("Load mods"), active=win.settings["mods_enabled"]
    )
    win.mods_group.add(win.mods_enabled_row)
    win.mods_folder_row = Adw.ActionRow(title=tr("Mods folder"))
    win.mods_folder_row.add_suffix(flat_button(
        "folder-open-symbolic", tr("Choose the mods folder"), win.on_choose_mods
    ))
    win.mods_folder_row.add_suffix(flat_button(
        "system-file-manager-symbolic", tr("Open the mods folder"),
        lambda _b: open_folder(win, win.mods_dir())
    ))
    win.mods_folder_row.add_suffix(flat_button(
        "view-refresh-symbolic", tr("Refresh the list"), win.on_refresh_mods
    ))
    win.mods_group.add(win.mods_folder_row)
    win.mod_list = FolderList(win.mods_group)
    win.refresh_mods()
    return win.mods_group


def build_patches_group(win):
    desc = tr(
        "shadPS4-format XML patches for version 01.09 from the patches folder. "
        "Applied at start."
    )
    win.patches_group = Adw.PreferencesGroup(title=tr("Third-party patches"), description=desc)
    win.patches_folder_row = Adw.ActionRow(title=tr("Patches folder"))
    win.patches_folder_row.add_suffix(flat_button(
        "folder-open-symbolic", tr("Choose the patches folder"), win.on_choose_patches
    ))
    win.patches_folder_row.add_suffix(flat_button(
        "system-file-manager-symbolic", tr("Open the patches folder"),
        lambda _b: open_folder(win, patches_dir(win.settings))
    ))
    win.patches_folder_row.add_suffix(flat_button(
        "view-refresh-symbolic", tr("Refresh the list"), win.on_refresh_patches
    ))
    win.patches_group.add(win.patches_folder_row)
    win.patch_list = FolderList(win.patches_group)
    win.refresh_patches()
    return win.patches_group
