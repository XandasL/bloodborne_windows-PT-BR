@echo off
setlocal
cd /d "%~dp0"

rem One click: launch the saved Bloodborne configuration and log NVIDIA GPU telemetry.
rem Monitor lives in tools\ in full packages, or next to this BAT in the CI test artifact.
set "MONITOR=%~dp0tools\monitor-gpu.ps1"
if not exist "%MONITOR%" set "MONITOR=%~dp0monitor-gpu.ps1"
if not exist "%MONITOR%" (
    echo Nao foi encontrado monitor-gpu.ps1.
    echo Copie o script para esta pasta ou para a subpasta tools.
    pause
    exit /b 1
)

powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%MONITOR%" -LaunchGame -IntervalSeconds 2 -OutputDirectory "%~dp0user\gpu_logs"
set "RESULT=%ERRORLEVEL%"
if not "%RESULT%"=="0" (
    echo.
    echo A sessao terminou com erro. Verifique user\last_run.log e user\gpu_logs\.
    pause
)
exit /b %RESULT%
