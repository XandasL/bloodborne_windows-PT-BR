# Captura temperatura, consumo, utilizacao e ventoinhas durante testes do Bloodborne.
# Execute em um PowerShell separado. Pare com Ctrl+C depois de sair do jogo.
param(
    [ValidateRange(1, 60)]
    [int]$IntervalSeconds = 2,
    [string]$OutputPath = ""
)

$ErrorActionPreference = "Stop"
$command = Get-Command nvidia-smi.exe -ErrorAction SilentlyContinue
if ($command) {
    $nvidiaSmi = $command.Source
} else {
    $candidates = @(
        (Join-Path $env:ProgramFiles "NVIDIA Corporation\NVSMI\nvidia-smi.exe"),
        (Join-Path $env:WINDIR "System32\nvidia-smi.exe")
    )
    $nvidiaSmi = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}
if (-not $nvidiaSmi) {
    throw "nvidia-smi.exe nao encontrado. Use o HWiNFO64 (Sensors -> Start Logging) como alternativa."
}

if (-not $OutputPath) {
    $OutputPath = Join-Path $PSScriptRoot ("gpu_monitor_" + (Get-Date -Format "yyyyMMdd_HHmmss") + ".csv")
}
$OutputPath = $ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($OutputPath)
$fields = "timestamp,temperature.gpu,utilization.gpu,memory.used,power.draw,fan.speed,clocks.gr"
Write-Host "Monitorando GPU NVIDIA a cada $IntervalSeconds segundos."
Write-Host "Log CSV: $OutputPath"
Write-Host "Jogue normalmente. Depois do jogo/crash, volte aqui e pressione Ctrl+C."
& $nvidiaSmi "--query-gpu=$fields" "--format=csv" "--loop=$IntervalSeconds" "--filename=$OutputPath"
