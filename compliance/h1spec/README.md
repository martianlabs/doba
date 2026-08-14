# h1spec compliance adapter

This target exposes the routes h1spec uses to probe a HTTP/1.1 server. It is
independent of the doba unit-test, Http11Probe compliance, and HttpArena
benchmark targets.

## Requirements

- CMake 3.20 or newer and a C++20 compiler.
- Docker Desktop or Docker Engine with Docker Compose V2.
- Network access when the runner image is first built.
- On Windows, a Visual Studio developer shell when building with MSVC.

The runner downloads h1spec during its image build, checks out commit
`f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e`, and verifies that checkout. No
local clone of h1spec is required.

## Run the suite

Build the adapter in Release mode:

```text
cmake -S compliance/h1spec -B build/h1spec -DCMAKE_BUILD_TYPE=Release
cmake --build build/h1spec --config Release
```

In Windows PowerShell, start the adapter, run the runner, and always stop the
adapter when the suite exits:

```powershell
$server = Start-Process `
  -FilePath (Resolve-Path "build/h1spec/Release/doba_h1spec.exe") `
  -WindowStyle Hidden `
  -PassThru
$exitCode = 1
try {
  Start-Sleep -Seconds 1
  docker compose -f compliance/h1spec/docker-compose.yml run --rm --build h1spec
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
`build/h1spec/doba_h1spec` rather than the MSVC Release path above. Run only
one compliance suite at a time because both default to port `8080`.

h1spec writes its complete result to the terminal and returns a non-zero exit
status when any test fails; it does not create a result artifact.

## Verified result

On 2026-08-14, commit `f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e` passed all
33 h1spec tests against this adapter on Docker Desktop for Windows.
