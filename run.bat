@echo off
setlocal enabledelayedexpansion
cd /d "%~dp0"

set "DATA_DIR=%~dp0"
set "OUT_DIR=%DATA_DIR%out"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"
if not defined BB_CONFIG set "BB_CONFIG=%DATA_DIR%bbport.ini"
if not defined BB_MODS_DIR set "BB_MODS_DIR=%DATA_DIR%mods"
if not defined BB_MODS_CONFIG set "BB_MODS_CONFIG=%DATA_DIR%mods.json"
if not defined BB_MODS_ENABLED set "BB_MODS_ENABLED=1"

if not defined BB_GAME_DIR (
    for %%C in (CUSA00900 CUSA03173 CUSA00207 CUSA00208 CUSA03023 CUSA01363) do (
        if not defined BB_GAME_DIR (
            if exist "%~dp0..\%%C\eboot.bin" set "BB_GAME_DIR=%~dp0..\%%C"
            if exist "%~dp0%%C\eboot.bin" set "BB_GAME_DIR=%~dp0%%C"
        )
    )
)
if not defined BB_GAME_DIR (
    echo [ERROR] No eboot.bin found. Set BB_GAME_DIR to your game directory.
    exit /b 1
)

where py >nul 2>&1 && (set "PYTHON_EXE=py") || (
    where python >nul 2>&1 && (set "PYTHON_EXE=python") || (
        echo [ERROR] Python 3 was not found in PATH.
        exit /b 1
    )
)

for %%I in ("%BB_GAME_DIR%") do set "ORIGINAL_GAME_DIR=%%~fI"
set "MOD_VIEW_FILE=%TEMP%\bbport-mod-%RANDOM%-%RANDOM%.txt"
echo [MODS] Building game view...
%PYTHON_EXE% scripts\mods.py "%ORIGINAL_GAME_DIR%" --out "%OUT_DIR%" ^
    --mods-dir "%BB_MODS_DIR%" --config "%BB_MODS_CONFIG%" --enabled "%BB_MODS_ENABLED%" > "%MOD_VIEW_FILE%"
if errorlevel 1 (
    del "%MOD_VIEW_FILE%" >nul 2>&1
    echo [ERROR] Mod overlay preparation failed.
    exit /b 1
)
set /p "BB_GAME_DIR="<"%MOD_VIEW_FILE%"
del "%MOD_VIEW_FILE%" >nul 2>&1
if /I not "%BB_GAME_DIR%"=="%ORIGINAL_GAME_DIR%" set "MOD_OVERLAY=%BB_GAME_DIR%"

echo [1/4] Preparing game binary image...
%PYTHON_EXE% scripts\prepare.py "%BB_GAME_DIR%" --out "%OUT_DIR%"
if errorlevel 1 goto :fail

echo [2/4] Linking libc exports...
%PYTHON_EXE% scripts\link_libc.py "%BB_GAME_DIR%" --out "%OUT_DIR%"
if errorlevel 1 goto :fail

echo [3/4] Linking guest modules...
%PYTHON_EXE% scripts\link_modules.py "%BB_GAME_DIR%" --out "%OUT_DIR%"
if errorlevel 1 goto :fail

echo [4/4] Generating offline content profile ^& patches...
%PYTHON_EXE% scripts\content_profile.py "%BB_GAME_DIR%" --out "%OUT_DIR%" --sku full
if errorlevel 1 goto :fail
%PYTHON_EXE% scripts\patches.py --out "%OUT_DIR%" --fps uncap --game-dir "%BB_GAME_DIR%"
if errorlevel 1 goto :fail

set "PROBE_EXE=%~dp0bb-probe.exe"
if not exist "%PROBE_EXE%" set "PROBE_EXE=%OUT_DIR%\bb-probe.exe"
if not exist "%PROBE_EXE%" set "PROBE_EXE=%~dp0build\bb-probe.exe"
if not exist "%PROBE_EXE%" (
    echo [ERROR] bb-probe.exe not found.
    goto :fail
)

echo [LAUNCH] Launching Bloodborne via bb-probe...
"%PROBE_EXE%" "%OUT_DIR%\boot-linked.bin" --content-profile "%OUT_DIR%\content.bin" ^
    --patches "%OUT_DIR%\patches.bin" --app0 "%BB_GAME_DIR%" --user "%DATA_DIR%user" %*
set "GAME_EXIT=%ERRORLEVEL%"
goto :cleanup

:fail
set "GAME_EXIT=1"

:cleanup
if defined MOD_OVERLAY rmdir /s /q "%MOD_OVERLAY%" >nul 2>&1
exit /b %GAME_EXIT%
