# h1spec compliance adapter

This target exposes the routes h1spec uses to probe a HTTP/1.1 server. It is
independent of the doba unit-test, Http11Probe compliance, and HttpArena
benchmark targets.

## Requirements

- CMake 3.20 or newer and a C++20 compiler.
- Docker Desktop or Docker Engine with Docker Compose V2.
- Network access when the runner image is first built.

The runner downloads h1spec during its image build, checks out commit
`f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e`, and verifies that checkout. No
local clone of h1spec is required.

Run only one compliance suite at a time because both default to port `8080`.

## Windows

Use a Visual Studio developer PowerShell. Run the following commands in the
same window. Run the cleanup command even if the suite fails.

Configure the Release build:

```powershell
cmake -S compliance/h1spec -B build/h1spec -DCMAKE_BUILD_TYPE=Release
```

Build the adapter:

```powershell
cmake --build build/h1spec --config Release
```

Start the adapter:

```powershell
$server = Start-Process -FilePath (Resolve-Path "build/h1spec/Release/doba_h1spec.exe") -WindowStyle Hidden -PassThru
```

Wait for the adapter to start:

```powershell
Start-Sleep -Seconds 1
```

Run the suite:

```powershell
docker compose -f compliance/h1spec/docker-compose.yml run --rm --build h1spec
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
cmake -S compliance/h1spec -B build/h1spec -DCMAKE_BUILD_TYPE=Release
```

Build the adapter:

```sh
cmake --build build/h1spec
```

Start the adapter and retain its process ID:

```sh
./build/h1spec/doba_h1spec & server_pid=$!
```

Wait for the adapter to start:

```sh
sleep 1
```

Run the suite:

```sh
docker compose -f compliance/h1spec/docker-compose.yml run --rm --build h1spec
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

h1spec writes its complete result to the terminal and returns a non-zero exit
status when any test fails; it does not create a result artifact.

## Verified result

On 2026-08-14, commit `f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e` passed all
33 h1spec tests against this adapter on Docker Desktop for Windows.
