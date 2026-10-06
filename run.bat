@echo off
setlocal enabledelayedexpansion

cd /d "%~dp0"

set "DATA_DIR=%~dp0"
set "OUT_DIR=%DATA_DIR%out"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

if not defined BB_CONFIG set "BB_CONFIG=%DATA_DIR%bbport.ini"
if not defined BB_GAME_DIR (
    for %%C in (CUSA00900 CUSA03173 CUSA00207 CUSA00208 CUSA03023 CUSA01363) do (
        if not defined BB_GAME_DIR (
            if exist "%~dp0..\%%C\eboot.bin" set "BB_GAME_DIR=%~dp0..\%%C"
            if exist "%~dp0%%C\eboot.bin" set "BB_GAME_DIR=%~dp0%%C"
        )
    )
    if not defined BB_GAME_DIR (
        echo [ERROR] No eboot.bin found. Please set BB_GAME_DIR to your game directory (e.g. CUSA00900 or CUSA03173).
        pause
        exit /b 1
    )
)

where py >nul 2>&1
if %errorlevel% equ 0 (
    set "PYTHON_EXE=py"
) else (
    where python >nul 2>&1
    if %errorlevel% equ 0 (
        set "PYTHON_EXE=python"
    ) else (
        echo [ERROR] Python 3 was not found in PATH. Please install Python 3.
        pause
        exit /b 1
    )
)

echo [1/4] Preparing game binary image...
%PYTHON_EXE% scripts\prepare.py "%BB_GAME_DIR%" --out "%OUT_DIR%"
if %errorlevel% neq 0 ( echo [ERROR] prepare.py failed. & pause & exit /b 1 )

echo [2/4] Linking libc exports...
%PYTHON_EXE% scripts\link_libc.py "%BB_GAME_DIR%" --out "%OUT_DIR%"
if %errorlevel% neq 0 ( echo [ERROR] link_libc.py failed. & pause & exit /b 1 )

echo [3/4] Linking guest modules...
%PYTHON_EXE% scripts\link_modules.py "%BB_GAME_DIR%" --out "%OUT_DIR%"
if %errorlevel% neq 0 ( echo [ERROR] link_modules.py failed. & pause & exit /b 1 )

echo [4/4] Generating offline content profile & patches...
%PYTHON_EXE% scripts\content_profile.py "%BB_GAME_DIR%" --out "%OUT_DIR%" --sku full
%PYTHON_EXE% scripts\patches.py --out "%OUT_DIR%" --fps uncap --game-dir "%BB_GAME_DIR%"

set "PROBE_EXE=%OUT_DIR%\bb-probe.exe"
if not exist "%PROBE_EXE%" (
    if exist "%~dp0build\bb-probe.exe" (
        set "PROBE_EXE=%~dp0build\bb-probe.exe"
    ) else (
        echo [ERROR] bb-probe.exe not found. Please build using CMake or your compiler first.
        pause
        exit /b 1
    )
)

echo [LAUNCH] Launching Bloodborne via bb-probe...
"%PROBE_EXE%" "%OUT_DIR%\boot-linked.bin" --content-profile "%OUT_DIR%\content.bin" --patches "%OUT_DIR%\patches.bin" --app0 "%BB_GAME_DIR%" --user "%DATA_DIR%user" %*
