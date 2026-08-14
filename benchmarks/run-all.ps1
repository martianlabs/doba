param(
  [ValidateSet("benchmark", "validate")]
  [string]$Mode = "benchmark",
  [string]$WebFrameworks = "",
  [string]$HttpArenaFrameworks = ""
)

$ErrorActionPreference = "Stop"
$powerShellExecutable = (Get-Process -Id $PID).Path
$webFrameworksScript = Join-Path $PSScriptRoot "run-web-frameworks.ps1"
$httpArenaScript = Join-Path $PSScriptRoot "run-httparena.ps1"

$webArguments = @(
  "-NoProfile",
  "-ExecutionPolicy", "Bypass",
  "-File", $webFrameworksScript,
  "-Mode", $Mode
)
if ($WebFrameworks) {
  $webArguments += @("-Frameworks", $WebFrameworks)
}

$httpArenaArguments = @(
  "-NoProfile",
  "-ExecutionPolicy", "Bypass",
  "-File", $httpArenaScript,
  "-Mode", $Mode
)
if ($HttpArenaFrameworks) {
  $httpArenaArguments += @("-Frameworks", $HttpArenaFrameworks)
}

Write-Host "`n=== Web Frameworks ==="
& $powerShellExecutable @webArguments
$webFrameworksExitCode = $LASTEXITCODE

Write-Host "`n=== HttpArena ==="
& $powerShellExecutable @httpArenaArguments
$httpArenaExitCode = $LASTEXITCODE

if ($webFrameworksExitCode -ne 0 -or $httpArenaExitCode -ne 0) {
  exit 1
}

exit 0
