<#
.SYNOPSIS
    Bloodborne Windows Native Runner Script (PowerShell)
.DESCRIPTION
    Prepares the decrypted PS4 CUSA03173 Bloodborne assets and launches bb-probe.exe natively.
#>

[CmdletBinding()]
param(
    [string]$GameDir = $env:BB_GAME_DIR,
    [string]$DataDir = $PSScriptRoot,
    [string]$Fps = "uncap",
    [switch]$SmokeTest
)

$ErrorActionPreference = "Stop"
Set-Location $PSScriptRoot

if ($SmokeTest) {
    Write-Host "[SMOKE TEST] Running Vulkan smoke test..." -ForegroundColor Cyan
    $probe = if (Test-Path "$PSScriptRoot\out\bb-probe.exe") { "$PSScriptRoot\out\bb-probe.exe" } else { "$PSScriptRoot\build\bb-probe.exe" }
    if (-not (Test-Path $probe)) {
        Write-Error "bb-probe.exe not found. Build the project first."
    }
    & $probe --vulkan-only
    exit $LASTEXITCODE
}

$outDir = Join-Path $DataDir "out"
if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }

if ([string]::IsNullOrWhiteSpace($GameDir)) {
    if (Test-Path "$PSScriptRoot\..\CUSA03173\eboot.bin") {
        $GameDir = (Resolve-Path "$PSScriptRoot\..\CUSA03173").Path
    } elseif (Test-Path "$PSScriptRoot\CUSA03173\eboot.bin") {
        $GameDir = (Resolve-Path "$PSScriptRoot\CUSA03173").Path
    } else {
        Write-Error "No eboot.bin found. Please specify -GameDir <path-to-CUSA03173> or set BB_GAME_DIR."
    }
}

$py = Get-Command py -ErrorAction SilentlyContinue
if (-not $py) { $py = Get-Command python -ErrorAction SilentlyContinue }
if (-not $py) { Write-Error "Python 3 is required. Please install Python 3 and add it to PATH." }

Write-Host "[1/4] Preparing game binary image..." -ForegroundColor Green
& $py.Source scripts/prepare.py $GameDir --out $outDir

Write-Host "[2/4] Linking libc exports..." -ForegroundColor Green
& $py.Source scripts/link_libc.py $GameDir --out $outDir

Write-Host "[3/4] Linking guest modules..." -ForegroundColor Green
& $py.Source scripts/link_modules.py $GameDir --out $outDir

Write-Host "[4/4] Generating offline content profile & patches..." -ForegroundColor Green
& $py.Source scripts/content_profile.py $GameDir --out $outDir --sku full
& $py.Source scripts/patches.py --out $outDir --fps $Fps --game-dir $GameDir

$probeExe = if (Test-Path "$outDir\bb-probe.exe") { "$outDir\bb-probe.exe" } else { "$PSScriptRoot\build\bb-probe.exe" }
if (-not (Test-Path $probeExe)) {
    Write-Error "bb-probe.exe not found. Please build using CMake or your compiler first."
}

$userDir = Join-Path $DataDir "user"
Write-Host "[LAUNCH] Launching Bloodborne via bb-probe..." -ForegroundColor Cyan
& $probeExe "$outDir\boot-linked.bin" --content-profile "$outDir\content.bin" --patches "$outDir\patches.bin" --app0 $GameDir --user $userDir
