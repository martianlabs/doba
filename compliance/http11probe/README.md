# Http11Probe compliance adapter

This target exposes the routes Http11Probe uses to probe a HTTP/1.1 server. It
is independent of the doba unit-test and HttpArena benchmark targets.

## Requirements

- CMake 3.20 or newer and a C++20 compiler.
- Docker Desktop or Docker Engine with Docker Compose V2.
- Network access when the runner image is first built.

The runner downloads Http11Probe during its image build, checks out commit
`59513c793ca1f148c82cfedc4b5d75b81d47c591`, restores and builds its CLI. No
local clone of Http11Probe is required.

Run only one compliance suite at a time because both default to port `8080`.

## Windows

Use a Visual Studio developer PowerShell. Run the following commands in the
same window. Run the cleanup command even if the suite fails.

Configure the Release build:

```powershell
cmake -S compliance/http11probe -B build/http11probe -DCMAKE_BUILD_TYPE=Release
```

Build the adapter:

```powershell
cmake --build build/http11probe --config Release
```

Start the adapter:

```powershell
$server = Start-Process -FilePath (Resolve-Path "build/http11probe/Release/doba_http11probe.exe") -WindowStyle Hidden -PassThru
```

Wait for the adapter to start:

```powershell
Start-Sleep -Seconds 1
```

Run the suite:

```powershell
docker compose -f compliance/http11probe/docker-compose.yml run --rm --build http11probe
```

Stop the adapter:

```powershell
Stop-Process -Id $server.Id -Force
```

## Linux

Run the following commands in the same shell. Run the cleanup command even if
the suite fails.

Configure the Release build:

```sh
cmake -S compliance/http11probe -B build/http11probe -DCMAKE_BUILD_TYPE=Release
```

Build the adapter:

```sh
cmake --build build/http11probe
```

Start the adapter and retain its process ID:

```sh
./build/http11probe/doba_http11probe & server_pid=$!
```

Wait for the adapter to start:

```sh
sleep 1
```

Run the suite:

```sh
docker compose -f compliance/http11probe/docker-compose.yml run --rm --build http11probe
```

Stop the adapter:

```sh
kill "$server_pid"
```

## Target override

The runner targets `host.docker.internal:8080`. Override either endpoint part
when necessary before invoking Docker Compose. `DOBA_HOST` must resolve from
inside the runner container.

```powershell
$env:DOBA_HOST = "server.internal"
```

```powershell
$env:DOBA_PORT = "8081"
```

```sh
export DOBA_HOST=server.internal
```

```sh
export DOBA_PORT=8081
```

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
