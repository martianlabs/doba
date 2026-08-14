param(
  [string]$DobaHost = "host.docker.internal",
  [int]$DobaPort = 8080
)

$ErrorActionPreference = "Stop"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$buildDirectory = Join-Path $repoRoot "build\http11probe"

cmake -S $PSScriptRoot -B $buildDirectory -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cmake --build $buildDirectory --config Release
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$serverPath = Join-Path $buildDirectory "Release\doba_http11probe.exe"
if (-not (Test-Path $serverPath)) { throw "Adapter executable not found: $serverPath" }

$hadDobaHost = Test-Path Env:DOBA_HOST
$hadDobaPort = Test-Path Env:DOBA_PORT
$previousDobaHost = $env:DOBA_HOST
$previousDobaPort = $env:DOBA_PORT
$env:DOBA_HOST = $DobaHost
$env:DOBA_PORT = $DobaPort
$server = $null
$exitCode = 1

try {
  $server = Start-Process -FilePath $serverPath -WindowStyle Hidden -PassThru
  Start-Sleep -Seconds 1
  docker compose -f (Join-Path $PSScriptRoot "docker-compose.yml") run --rm --build http11probe
  $exitCode = $LASTEXITCODE
}
finally {
  if ($server -and -not $server.HasExited) {
    Stop-Process -Id $server.Id -Force -ErrorAction SilentlyContinue
    $server.WaitForExit()
  }
  if ($hadDobaHost) {
    $env:DOBA_HOST = $previousDobaHost
  } else {
    Remove-Item Env:DOBA_HOST
  }
  if ($hadDobaPort) {
    $env:DOBA_PORT = $previousDobaPort
  } else {
    Remove-Item Env:DOBA_PORT
  }
}

exit $exitCode
