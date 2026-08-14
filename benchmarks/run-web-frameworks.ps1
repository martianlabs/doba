param(
  [ValidateSet("benchmark", "validate")]
  [string]$Mode = "benchmark",
  [string]$Frameworks = ""
)

$ErrorActionPreference = "Stop"
$composePath = Join-Path $PSScriptRoot "compose.yaml"
$arguments = @("compose", "-f", $composePath, "run", "--rm", "--build")
if ($Frameworks) {
  $arguments += @("-e", "WEB_FRAMEWORKS=$Frameworks")
}
$arguments += @("runner", "web-frameworks", $Mode)
$exitCode = 1

try {
  & docker @arguments
  $exitCode = $LASTEXITCODE
}
finally {
  & docker compose -f $composePath down
  if ($exitCode -eq 0 -and $LASTEXITCODE -ne 0) {
    $exitCode = $LASTEXITCODE
  }
}

exit $exitCode
