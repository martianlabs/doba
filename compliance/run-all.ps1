param(
  [string]$DobaHost = "host.docker.internal",
  [int]$DobaPort = 8080
)

$ErrorActionPreference = "Stop"
$utf8Encoding = [System.Text.UTF8Encoding]::new($false)
[Console]::InputEncoding = $utf8Encoding
[Console]::OutputEncoding = $utf8Encoding
$OutputEncoding = $utf8Encoding
$timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$outputDirectory = Join-Path $PSScriptRoot "out\all-$timestamp"
$summaryPath = Join-Path $outputDirectory "summary.txt"
$h1specLog = Join-Path $outputDirectory "h1spec.log"
$http11ProbeLog = Join-Path $outputDirectory "http11probe.log"
$h1specScript = Join-Path $PSScriptRoot "run-h1spec.ps1"
$http11ProbeScript = Join-Path $PSScriptRoot "run-http11probe.ps1"
$powerShellExecutable = (Get-Process -Id $PID).Path

New-Item -ItemType Directory -Force -Path $outputDirectory | Out-Null
$summaryLines = [System.Collections.Generic.List[string]]::new()

function Invoke-ComplianceSuite {
  param(
    [string]$Name,
    [string]$Script,
    [string]$Log
  )

  Write-Host "`n=== $Name ==="
  $ErrorActionPreference = "Continue"
  & $powerShellExecutable -NoProfile -ExecutionPolicy Bypass -File $Script -DobaHost $DobaHost -DobaPort $DobaPort 2>&1 |
    ForEach-Object {
      if ($_ -is [System.Management.Automation.ErrorRecord]) {
        $_.Exception.Message
      } else {
        $_
      }
    } |
    Tee-Object -FilePath $Log |
    Out-Host
  return $LASTEXITCODE
}

function Write-ComplianceSummary {
  param([string]$Line)

  Write-Host $Line
  $summaryLines.Add($Line)
}

$h1specExitCode = Invoke-ComplianceSuite -Name "h1spec" -Script $h1specScript -Log $h1specLog
$http11ProbeExitCode = Invoke-ComplianceSuite -Name "Http11Probe" -Script $http11ProbeScript -Log $http11ProbeLog

Write-ComplianceSummary ""
Write-ComplianceSummary "Compliance execution summary"
Write-ComplianceSummary "Target: $DobaHost`:$DobaPort"
$h1specResult = Select-String -Path $h1specLog -Pattern "(\d+) out of (\d+) tests passed\." |
  Select-Object -Last 1
$h1specPassed = $null
$h1specFailed = $null
$h1specTotal = $null
if ($h1specResult) {
  $h1specPassed = [int]$h1specResult.Matches[0].Groups[1].Value
  $h1specTotal = [int]$h1specResult.Matches[0].Groups[2].Value
  $h1specFailed = $h1specTotal - $h1specPassed
  Write-ComplianceSummary ""
  Write-ComplianceSummary "h1spec"
  Write-ComplianceSummary "  Total: $h1specTotal | Passed: $h1specPassed | Failed: $h1specFailed | Warnings: 0"
  Write-ComplianceSummary "  Runner exit code: $h1specExitCode"
} else {
  Write-ComplianceSummary ""
  Write-ComplianceSummary "h1spec"
  Write-ComplianceSummary "  No complete result (runner exit code: $h1specExitCode)"
}

$reportPath = Join-Path $PSScriptRoot "http11probe\out\http11probe\results.json"
$report = $null
$http11ProbeTotal = $null
$http11ProbePassed = $null
$http11ProbeFailed = $null
$http11ProbeWarnings = $null
$http11ProbeErrors = $null
if ($http11ProbeExitCode -eq 0 -and (Test-Path $reportPath)) {
  $report = Get-Content -Raw $reportPath | ConvertFrom-Json
  $summary = $report.summary
  $http11ProbeTotal = @($report.results).Count
  $http11ProbePassed = @($report.results | Where-Object { $_.verdict -eq "Pass" }).Count
  $http11ProbeFailed = @($report.results | Where-Object { $_.verdict -eq "Fail" }).Count
  $http11ProbeWarnings = @($report.results | Where-Object { $_.verdict -eq "Warn" }).Count
  $http11ProbeErrors = $summary.errors
  $scoredPassed = @($report.results | Where-Object { $_.scored -and $_.verdict -eq "Pass" }).Count
  $scoredFailed = @($report.results | Where-Object { $_.scored -and $_.verdict -eq "Fail" }).Count
  $scoredWarnings = @($report.results | Where-Object { $_.scored -and $_.verdict -eq "Warn" }).Count
  $unscoredPassed = @($report.results | Where-Object { -not $_.scored -and $_.verdict -eq "Pass" }).Count
  $unscoredFailed = @($report.results | Where-Object { -not $_.scored -and $_.verdict -eq "Fail" }).Count
  $unscoredWarnings = @($report.results | Where-Object { -not $_.scored -and $_.verdict -eq "Warn" }).Count

  Write-ComplianceSummary ""
  Write-ComplianceSummary "Http11Probe"
  Write-ComplianceSummary "  Total: $http11ProbeTotal | Passed: $http11ProbePassed | Failed: $http11ProbeFailed | Warnings: $http11ProbeWarnings | Errors: $http11ProbeErrors"
  Write-ComplianceSummary "  Scored: Passed: $scoredPassed | Failed: $scoredFailed | Warnings: $scoredWarnings"
  Write-ComplianceSummary "  Unscored: Passed: $unscoredPassed | Failed: $unscoredFailed | Warnings: $unscoredWarnings"
  Write-ComplianceSummary ("  Duration: {0:N1} ms | Runner exit code: {1}" -f $summary.durationMs, $http11ProbeExitCode)

  $scoredFailures = @($report.results | Where-Object { $_.scored -and $_.verdict -eq "Fail" })
  $unscoredFailures = @($report.results | Where-Object { -not $_.scored -and $_.verdict -eq "Fail" })
  if ($scoredFailures.Count -eq 0) {
    Write-ComplianceSummary "  Scored failures: none"
  } else {
    Write-ComplianceSummary "  Scored failures:"
    foreach ($failure in $scoredFailures) {
      $reference = if ($failure.rfcReference) { " [$($failure.rfcReference)]" } else { "" }
      Write-ComplianceSummary "    $($failure.id)$reference - $($failure.description)"
      Write-ComplianceSummary "      expected: $($failure.expected); received: $($failure.statusCode) / $($failure.connectionState)"
    }
  }

  if ($unscoredFailures.Count -ne 0) {
    Write-ComplianceSummary "  Unscored failures:"
    foreach ($failure in $unscoredFailures) {
      Write-ComplianceSummary "    $($failure.id) - $($failure.description)"
      Write-ComplianceSummary "      expected: $($failure.expected); received: $($failure.statusCode) / $($failure.connectionState)"
    }
  }
} else {
  Write-ComplianceSummary ""
  Write-ComplianceSummary "Http11Probe"
  Write-ComplianceSummary "  No current report (runner exit code: $http11ProbeExitCode)"
}

if ($null -ne $h1specTotal -and $null -ne $http11ProbeTotal) {
  $globalTotal = $h1specTotal + $http11ProbeTotal
  $globalPassed = $h1specPassed + $http11ProbePassed
  $globalFailed = $h1specFailed + $http11ProbeFailed
  $globalWarnings = $http11ProbeWarnings
  $globalErrors = $http11ProbeErrors

  Write-ComplianceSummary ""
  Write-ComplianceSummary "Global"
  Write-ComplianceSummary "  Tests executed: $globalTotal"
  Write-ComplianceSummary "  Passed: $globalPassed | Failed: $globalFailed | Warnings: $globalWarnings | Errors: $globalErrors"
}

Write-ComplianceSummary ""
Write-ComplianceSummary "Logs: $outputDirectory"
if ($report) {
  Write-ComplianceSummary "Http11Probe report: $reportPath"
}

$exitCode = 0
if ($h1specExitCode -ne 0 -or $http11ProbeExitCode -ne 0) {
  $exitCode = 1
}
if ($report -and ($report.summary.failed -ne 0 -or $report.summary.errors -ne 0)) {
  $exitCode = 1
}

if ($exitCode -eq 0) {
  Write-ComplianceSummary "Runner status: completed without scored failures or errors"
} else {
  Write-ComplianceSummary "Runner status: completed with test failures or runner errors"
}

[System.IO.File]::WriteAllLines($summaryPath, $summaryLines, [System.Text.UTF8Encoding]::new($false))

exit $exitCode
