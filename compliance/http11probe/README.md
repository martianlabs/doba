# Http11Probe compliance adapter

This target exposes the routes Http11Probe uses to probe a HTTP/1.1 server. It
is independent of the doba unit-test and HttpArena benchmark targets.

## Requirements

- CMake 3.20 or newer and a C++20 compiler.
- Docker Desktop or Docker Engine with Docker Compose V2.
- Network access when the runner image is first built.
- On Windows, a Visual Studio developer shell when building with MSVC.

The runner downloads Http11Probe during its image build, checks out commit
`59513c793ca1f148c82cfedc4b5d75b81d47c591`, restores and builds its CLI. No
local clone of Http11Probe is required.

## Run the suite

Build the adapter in Release mode:

```text
cmake -S compliance/http11probe -B build/http11probe -DCMAKE_BUILD_TYPE=Release
cmake --build build/http11probe --config Release
```

In Windows PowerShell, start the adapter, run the runner, and always stop the
adapter when the suite exits:

```powershell
$server = Start-Process `
  -FilePath (Resolve-Path "build/http11probe/Release/doba_http11probe.exe") `
  -WindowStyle Hidden `
  -PassThru
$exitCode = 1
try {
  Start-Sleep -Seconds 1
  docker compose -f compliance/http11probe/docker-compose.yml run --rm --build http11probe
  $exitCode = $LASTEXITCODE
}
finally {
  if (-not $server.HasExited) {
    Stop-Process -Id $server.Id -Force
    $server.WaitForExit()
  }
}
exit $exitCode
```

The runner targets `host.docker.internal:8080`. Override either endpoint part
when necessary before invoking Docker Compose. `DOBA_HOST` must resolve from
inside the runner container:

```powershell
$env:DOBA_HOST = "server.internal"
$env:DOBA_PORT = "8081"
```

For a single-config generator, the executable is normally
`build/http11probe/doba_http11probe` rather than the MSVC Release path above.
Run only one compliance suite at a time because both default to port `8080`.

Http11Probe writes `results.json` to
`compliance/http11probe/out/http11probe/results.json`. This directory is
ignored by Git; inspect, archive, or remove the artifact after the run as
needed. The CLI can exit with status zero when the report contains failed
cases, so use `summary.failed` and the individual verdicts in `results.json`
as the pass/fail result.

## Verified result

On 2026-08-14, the pinned runner completed 213 probes against this adapter.
The report scored 157/159 tests as passed, with 2 scored failures and 11
warnings. See the generated JSON for the individual cases.
