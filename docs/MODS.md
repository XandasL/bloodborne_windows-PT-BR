# File Mods

The launcher provides a **Mods** section: mod folder selection, master toggle, individual mod toggles, and load order arrows. Changes take effect on the next launch. The lowest enabled mod in the list has the highest priority; in case of file collisions, the log displays which mod replaced earlier files.

By default, `bbport/mods/` is used (on AppImage: `~/.local/share/bbport/mods/`). Extract downloaded mods as follows:

```text
mods/
  Mod Name/
    dvdroot_ps4/
      chr/...
      param/...
      menu/...
```

The following structures are also accepted:

- `Mod Name/app0/dvdroot_ps4/...` and `Mod Name/CUSA03173/dvdroot_ps4/...`;
- An extra wrapper directory created by archive extraction: `Mod Name/Mod Name v1.2/dvdroot_ps4/...` (alongside readme files/images);
- Direct mod directory without `dvdroot_ps4` containing game asset folders: `Mod Name/chr/...`, `Mod Name/parts/...` (`chr`, `parts`, `map`, `menu`, `msg`, `param`, `sfx`, `sound`, `font`, and other `dvdroot_ps4` subdirectories).

Filename casing is case-insensitive: `DVDROOT_PS4/Chr/C0000.chrbnd.dcx` from a Windows mod correctly replaces game `dvdroot_ps4/chr/c0000.chrbnd.dcx`. The launch log indicates how many files were replaced and added: `Mods: 12 game files replaced, 0 added`. If a mod is intended to replace files but the log reports `0 game files replaced`, check directory paths within the mod folder. ZIP/7z archives must be extracted first. Standard `dvdroot_ps4` replacements are supported: textures, models, parameters, UI, audio, and fonts. The mod manager does not merge contents of two colliding `.dcx`/`.bnd` files: the last override wins completely.

For existing shadPS4 installations, the sibling folder `CUSA03173-mods/dvdroot_ps4/...` next to `CUSA03173/` is automatically recognized. It is mounted before mods from the list. The master toggle also disables this folder. Alternatively, a folder directly containing `dvdroot_ps4` can be chosen as the mods folder, mounted whole without a list.

Original game files are never overwritten. At runtime, an isolated union mount/symlink overlay tree is created under data directory `out/mod-game-*`. File operations (read, stat, directory listing) see selected mod overrides and remaining original files. Game directories `/app0` and `/hostapp` are mounted read-only. Save games remain in their dedicated save folder and are read-write. On normal termination or launcher exit, temporary overlay trees are cleaned up automatically.

Mods modifying `eboot.bin`, `sce_module`, or `sce_sys`, DLL injectors, and script hooks are not supported by this file loader. Executable code patches are handled via `patches/` / `BB_PATCHES`. Symbolic links inside mods are not followed.

To launch from terminal:

```sh
BB_GAME_DIR=/path/to/CUSA03173 BB_MODS_DIR=/path/to/mods bash run.sh
BB_GAME_DIR=/path/to/CUSA03173 BB_MODS_ENABLED=0 bash run.sh
```

Mod load order and disabled status are saved in data directory `mods.json`:

```json
{"order": ["First", "Second"], "disabled": ["First"]}
```

New folders are automatically enabled and appended alphabetically. `BB_MODS_CONFIG` overrides the profile path. `BB_MOD_TRACE=1` alongside `BB_MODS_DIR` logs initial file reads for debugging.

# Third-Party Patches (shadPS4 XML)

The `patches/` folder in the data directory (AppImage: `~/.local/share/bbport/patches/`, or configured via the launcher **Third-party Patches** section) accepts shadPS4/GoldHEN XML patch files. Patches targeting `AppVer="01.09"` and `eboot.bin` are loaded; patches for other TitleIDs are skipped. Default enable state comes from the `isEnabled` attribute in the file; launcher selections are saved to `patches.json`:

```json
{"enabled": ["file.xml/Patch Name"], "disabled": ["file.xml/Other Patch"]}
```

Supported patch types include `bytes`, `bytes16/32/64`, `float32/64`, `utf8`, and `utf16`. Patches containing `mask` lines or addresses outside the eboot image range are skipped with a warning log. Third-party patches are applied after built-in patches (`patches/Bloodborne.xml`): on conflict, third-party patches take precedence. From terminal: `BB_PATCHES_DIR=/path BB_PATCHES_CONFIG=/path/patches.json bash run.sh`.

Patches relocating pointers in tables (such as `60 FPS++` and `90 FPS++`) are automatically re-based to the actual image load address by the runtime loader.

# TAA Sharpness

The `sharpen` and `sharpness` settings apply to TAA in both the launcher and in-game menu. Range is **0.0 - 1.0**; when set to 0 or disabled, no extra pass is run. AMD FidelityFX RCAS is applied to the resolved scene prior to HUD composition. Pre-sharpened color is stored in the TAA history buffer to prevent recursive over-sharpening over multiple frames. Flat color regions preserve original values including HDR without requiring auxiliary buffers.
