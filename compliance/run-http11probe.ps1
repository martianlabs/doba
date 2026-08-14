param(
  [string]$DobaHost = "host.docker.internal",
  [int]$DobaPort = 8080
)

$ErrorActionPreference = "Stop"
$timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$outputDirectory = Join-Path $PSScriptRoot "out\http11probe-$timestamp"
$summaryPath = Join-Path $outputDirectory "summary.txt"
$logPath = Join-Path $outputDirectory "http11probe.log"
$repoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$suiteDirectory = Join-Path $PSScriptRoot "http11probe"
$buildDirectory = Join-Path $repoRoot "build\http11probe-windows"

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
  $previousErrorActionPreference = $ErrorActionPreference
  $ErrorActionPreference = "Continue"
  try {
    docker compose -f (Join-Path $suiteDirectory "docker-compose.yml") run --rm --build http11probe 2>&1 |
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

$reportPath = Join-Path $suiteDirectory "out\http11probe\results.json"
Write-ComplianceSummary ""
Write-ComplianceSummary "Http11Probe execution summary"
Write-ComplianceSummary "Target: $DobaHost`:$DobaPort"
if ($exitCode -eq 0 -and (Test-Path $reportPath)) {
  $report = Get-Content -Raw $reportPath | ConvertFrom-Json
  $summary = $report.summary
  $passed = @($report.results | Where-Object { $_.verdict -eq "Pass" }).Count
  $failed = @($report.results | Where-Object { $_.verdict -eq "Fail" }).Count
  $warnings = @($report.results | Where-Object { $_.verdict -eq "Warn" }).Count
  $scoredPassed = @($report.results | Where-Object { $_.scored -and $_.verdict -eq "Pass" }).Count
  $scoredFailed = @($report.results | Where-Object { $_.scored -and $_.verdict -eq "Fail" }).Count
  $scoredWarnings = @($report.results | Where-Object { $_.scored -and $_.verdict -eq "Warn" }).Count
  $unscoredPassed = @($report.results | Where-Object { -not $_.scored -and $_.verdict -eq "Pass" }).Count
  $unscoredFailed = @($report.results | Where-Object { -not $_.scored -and $_.verdict -eq "Fail" }).Count
  $unscoredWarnings = @($report.results | Where-Object { -not $_.scored -and $_.verdict -eq "Warn" }).Count
  Write-ComplianceSummary "  Total: $(@($report.results).Count) | Passed: $passed | Failed: $failed | Warnings: $warnings | Errors: $($summary.errors)"
  Write-ComplianceSummary "  Scored: Passed: $scoredPassed | Failed: $scoredFailed | Warnings: $scoredWarnings"
  Write-ComplianceSummary "  Unscored: Passed: $unscoredPassed | Failed: $unscoredFailed | Warnings: $unscoredWarnings"
} else {
  Write-ComplianceSummary "  No complete test result"
}
Write-ComplianceSummary "  Runner exit code: $exitCode"
Write-ComplianceSummary "Logs: $logPath"
Write-ComplianceSummary "Http11Probe report: $reportPath"
[System.IO.File]::WriteAllLines($summaryPath, $summaryLines, [System.Text.UTF8Encoding]::new($false))

exit $exitCode
