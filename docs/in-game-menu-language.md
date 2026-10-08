# In-game settings language (Insert / L3+R3)

The overlay now follows the **Launcher language** choice from the Windows launcher.

- `ui_language=pt`: Portuguese (Brazilian UI)
- `ui_language=ru`: Russian
- `ui_language=en`: English
- `ui_language` empty / System: uses the Windows interface language
- Languages without an overlay translation (including other launcher languages): English fallback

The launcher passes the selected language through `BB_UI_LANGUAGE` when starting the
port, including "Play Bloodborne.exe" shortcuts and restarts. Direct `bb-probe.exe`
launches use the system UI language unless `BB_UI_LANGUAGE` is set explicitly.
The in-game overlay is translated, not the game assets or the launcher.

The overlay catalog lives in `gpu/shim/bbport_locale.h`. Add a language to this
table and its language resolver rather than embedding translated strings in the
widgets. Internal settings keys (`bbport.ini`) and game patch identifiers are unchanged.
