param(
  [string]$DobaHost = "host.docker.internal",
  [int]$DobaPort = 8080
)

$ErrorActionPreference = "Stop"
$timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$outputDirectory = Join-Path $PSScriptRoot "out\h1spec-$timestamp"
$summaryPath = Join-Path $outputDirectory "summary.txt"
$logPath = Join-Path $outputDirectory "h1spec.log"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$suiteDirectory = Join-Path $PSScriptRoot "h1spec"
$buildDirectory = Join-Path $repoRoot "build\h1spec-windows"

New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
$summaryLines = [System.Collections.Generic.List[string]]::new()

function Write-ComplianceSummary {
  param([string]$Line)

  Write-Host $Line
  $summaryLines.Add($Line)
}

cmake -S $suiteDirectory -B $buildDirectory -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

cmake --build $buildDirectory --config Release
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$serverPath = Join-Path $buildDirectory "Release\doba_h1spec.exe"
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
  $previousErrorActionPreference = $ErrorActionPreference
  $ErrorActionPreference = "Continue"
  try {
    docker compose -f (Join-Path $suiteDirectory "docker-compose.yml") run --rm --build h1spec 2>&1 |
      ForEach-Object {
        if ($_ -is [System.Management.Automation.ErrorRecord]) {
          $_.Exception.Message
        } else {
          $_
        }
      } |
      Tee-Object -FilePath $logPath |
      Out-Host
    $exitCode = $LASTEXITCODE
  }
  finally {
    $ErrorActionPreference = $previousErrorActionPreference
  }
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

$h1specResult = Select-String -Path $logPath -Pattern "(\d+) out of (\d+) tests passed\." |
  Select-Object -Last 1
Write-ComplianceSummary ""
Write-ComplianceSummary "h1spec execution summary"
Write-ComplianceSummary "Target: $DobaHost`:$DobaPort"
if ($h1specResult) {
  $passed = [int]$h1specResult.Matches[0].Groups[1].Value
  $total = [int]$h1specResult.Matches[0].Groups[2].Value
  $failed = $total - $passed
  Write-ComplianceSummary "  Total: $total | Passed: $passed | Failed: $failed | Warnings: 0"
} else {
  Write-ComplianceSummary "  No complete test result"
}
Write-ComplianceSummary "  Runner exit code: $exitCode"
Write-ComplianceSummary "Logs: $logPath"
[System.IO.File]::WriteAllLines($summaryPath, $summaryLines, [System.Text.UTF8Encoding]::new($false))

exit $exitCode
