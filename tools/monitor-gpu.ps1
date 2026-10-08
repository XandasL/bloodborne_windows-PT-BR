# Bloodborne / NVIDIA GPU telemetry. Uses nvidia-smi shipped with NVIDIA drivers.
# Manually: powershell -ExecutionPolicy Bypass -File .\monitor-gpu.ps1
# Automatically: Play Bloodborne + GPU Monitor.bat (next to Bloodborne.exe).
param(
    [ValidateRange(1, 60)]
    [int]$IntervalSeconds = 2,
    [string]$OutputPath = "",
    [string]$OutputDirectory = "",
    [switch]$LaunchGame,
    [ValidateRange(0, 15)]
    [int]$GpuIndex = 0
)

$ErrorActionPreference = "Stop"

function Find-NvidiaSmi {
    $command = Get-Command nvidia-smi.exe -ErrorAction SilentlyContinue
    if ($command) { return $command.Source }

    $paths = @(
        (Join-Path $env:ProgramFiles "NVIDIA Corporation\NVSMI\nvidia-smi.exe"),
        (Join-Path $env:WINDIR "System32\nvidia-smi.exe")
    )
    foreach ($path in $paths) {
        if (Test-Path -LiteralPath $path -PathType Leaf) { return $path }
    }
    return $null
}

# In the full package this script is under tools; the CI test artifact ships it at its root.
$portDir = if (Split-Path -Leaf $PSScriptRoot -eq "tools") {
    Split-Path -Parent $PSScriptRoot
} else {
    $PSScriptRoot
}

$gameFile = $null
$gameArguments = @()
if ($LaunchGame) {
    $playExe = Join-Path $portDir "Play Bloodborne.exe"
    $launcherExe = Join-Path $portDir "Bloodborne.exe"
    if (Test-Path -LiteralPath $playExe -PathType Leaf) {
        $gameFile = $playExe
    } elseif (Test-Path -LiteralPath $launcherExe -PathType Leaf) {
        $gameFile = $launcherExe
        $gameArguments = @("--play")
    } else {
        throw "Bloodborne.exe nao foi encontrado. Coloque o .bat na pasta do port, ao lado de Play Bloodborne.exe."
    }
}

$nvidiaSmi = Find-NvidiaSmi
if (-not $nvidiaSmi -and -not $LaunchGame) {
    throw "nvidia-smi.exe nao encontrado. Use o HWiNFO64 (Sensors > Start Logging) como alternativa."
}

if (-not $OutputPath) {
    if (-not $OutputDirectory) {
        $OutputDirectory = $PSScriptRoot
    }
    New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null
    $OutputPath = Join-Path $OutputDirectory ("gpu_monitor_" + (Get-Date -Format "yyyyMMdd_HHmmss_fff") + ".csv")
}
$OutputPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputPath)

$fields = "timestamp,temperature.gpu,utilization.gpu,memory.used,power.draw,fan.speed,clocks.gr"
$game = $null
$writer = $null
$gameExitCode = 0
try {
    if ($nvidiaSmi) {
        # Each sample is a separate query; every row is flushed immediately to survive crashes.
        # Avoid --loop: killing an outer PowerShell process could orphan a background nvidia-smi.
        $writer = [System.IO.StreamWriter]::new($OutputPath, $false, [System.Text.UTF8Encoding]::new($false))
        $writer.WriteLine($fields)
        $writer.Flush()
        Write-Host "Monitor NVIDIA: GPU $GpuIndex, intervalo $IntervalSeconds s"
        Write-Host "CSV: $OutputPath"
    } else {
        Write-Warning "nvidia-smi.exe nao encontrado. O jogo sera iniciado sem o registro da GPU."
    }

    if ($LaunchGame) {
        Write-Host "Abrindo Bloodborne com as configuracoes salvas no launcher..."
        $options = @{
            FilePath = $gameFile
            WorkingDirectory = $portDir
            PassThru = $true
        }
        if ($gameArguments.Count -gt 0) { $options.ArgumentList = $gameArguments }
        $game = Start-Process @options
    } else {
        Write-Host "Coleta manual. Pressione Ctrl+C para encerrar."
    }

    if (-not $nvidiaSmi) {
        if ($game) {
            $game.WaitForExit()
            $gameExitCode = $game.ExitCode
        }
    } else {
        while ($true) {
            if ($game) {
                $game.Refresh()
                if ($game.HasExited) { break }
            }

            # -i targets just the selected GPU. Results stay in NVIDIA's CSV format.
            $rows = @(& $nvidiaSmi "--id=$GpuIndex" "--query-gpu=$fields" "--format=csv,noheader" 2>&1)
            if ($LASTEXITCODE -ne 0) {
                Write-Warning "Falha consultando nvidia-smi. Registro interrompido; o jogo continua."
                if ($game) {
                    $game.WaitForExit()
                    break
                }
                throw "Falha consultando nvidia-smi."
            }
            foreach ($row in $rows) {
                $line = [string]$row
                if ($line -and -not $line.StartsWith("NVIDIA-SMI")) {
                    $writer.WriteLine($line)
                }
            }
            $writer.Flush()
            Start-Sleep -Seconds $IntervalSeconds
        }
        if ($game) {
            $game.WaitForExit()
            $gameExitCode = $game.ExitCode
        }
    }
} finally {
    if ($writer) {
        $writer.Flush()
        $writer.Dispose()
    }
    if ($game) { $game.Dispose() }
}
if ($LaunchGame) {
    if ($nvidiaSmi) { Write-Host "Bloodborne encerrado. Registro salvo em: $OutputPath" }
    exit $gameExitCode
}
