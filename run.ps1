<#
.SYNOPSIS
    Bloodborne HLE Recompiler & Translation Runner Script (PowerShell)
.DESCRIPTION
    Prepares Bloodborne, applies loose-file mods through an overlay, and launches bb-probe.
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

function Find-Probe {
    foreach ($candidate in @(
        "$PSScriptRoot\bb-probe.exe",
        "$PSScriptRoot\out\bb-probe.exe",
        "$PSScriptRoot\build\bb-probe.exe"
    )) {
        if (Test-Path $candidate) { return $candidate }
    }
    return $null
}

if ($SmokeTest) {
    Write-Host "[SMOKE TEST] Running Vulkan smoke test..." -ForegroundColor Cyan
    $probe = Find-Probe
    if (-not $probe) { Write-Error "bb-probe.exe not found." }
    & $probe --vulkan-only
    exit $LASTEXITCODE
}

$outDir = Join-Path $DataDir "out"
New-Item -ItemType Directory -Path $outDir -Force | Out-Null

if ([string]::IsNullOrWhiteSpace($GameDir)) {
    foreach ($cand in @("CUSA00900","CUSA03173","CUSA00207","CUSA00208",
                        "CUSA03023","CUSA01363","CUSA00299","CUSA03014")) {
        foreach ($root in @($PSScriptRoot, (Split-Path $PSScriptRoot -Parent))) {
            if (Test-Path "$root\$cand\eboot.bin") {
                $GameDir = (Resolve-Path "$root\$cand").Path
                break
            }
        }
        if ($GameDir) { break }
    }
}
if ([string]::IsNullOrWhiteSpace($GameDir)) {
    Write-Error "No eboot.bin found. Specify -GameDir or set BB_GAME_DIR."
}

$py = Get-Command py -ErrorAction SilentlyContinue
if (-not $py) { $py = Get-Command python -ErrorAction SilentlyContinue }
if (-not $py) { Write-Error "Python 3 is required." }

$originalGame = (Resolve-Path $GameDir).Path
$modsDir = if ($env:BB_MODS_DIR) { $env:BB_MODS_DIR } else { Join-Path $DataDir "mods" }
$modsConfig = if ($env:BB_MODS_CONFIG) { $env:BB_MODS_CONFIG } else { Join-Path $DataDir "mods.json" }
$modsEnabled = if ($env:BB_MODS_ENABLED) { $env:BB_MODS_ENABLED } else { "1" }

Write-Host "[MODS] Building game view..." -ForegroundColor Green
$modsArgs = @(
    "scripts/mods.py", $originalGame, "--out", $outDir,
    "--mods-dir", $modsDir, "--config", $modsConfig, "--enabled", $modsEnabled
)
$gameView = (& $py.Source @modsArgs | Select-Object -Last 1).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($gameView)) {
    Write-Error "Mod overlay preparation failed."
}
$usingOverlay = $gameView -ne $originalGame

try {
    Write-Host "[1/4] Preparing game binary image..." -ForegroundColor Green
    & $py.Source scripts/prepare.py $gameView --out $outDir
    Write-Host "[2/4] Linking libc exports..." -ForegroundColor Green
    & $py.Source scripts/link_libc.py $gameView --out $outDir
    Write-Host "[3/4] Linking guest modules..." -ForegroundColor Green
    & $py.Source scripts/link_modules.py $gameView --out $outDir
    Write-Host "[4/4] Generating content profile & patches..." -ForegroundColor Green
    & $py.Source scripts/content_profile.py $gameView --out $outDir --sku full
    & $py.Source scripts/patches.py --out $outDir --fps $Fps --game-dir $gameView

    $probeExe = Find-Probe
    if (-not $probeExe) { Write-Error "bb-probe.exe not found." }
    $userDir = Join-Path $DataDir "user"
    Write-Host "[LAUNCH] Launching Bloodborne via bb-probe..." -ForegroundColor Cyan
    & $probeExe "$outDir\boot-linked.bin" --content-profile "$outDir\content.bin" --patches "$outDir\patches.bin" --app0 $gameView --user $userDir
    exit $LASTEXITCODE
}
finally {
    if ($usingOverlay -and (Test-Path $gameView)) {
        Remove-Item -LiteralPath $gameView -Recurse -Force -ErrorAction SilentlyContinue
    }
}
