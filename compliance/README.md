# Compliance suites

This directory contains the runners for doba's HTTP/1.1 compliance suites:

- `h1spec`, pinned to `f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e`.
- `Http11Probe`, pinned to `59513c793ca1f148c82cfedc4b5d75b81d47c591`.

Each suite keeps its own adapter, Dockerfile, and Compose file. The runners
start one adapter at a time on port `8080`, so the suites always run
sequentially.

## Requirements

- CMake 3.20 or newer and a C++20 compiler.
- Docker Desktop or Docker Engine with Docker Compose V2.
- Network access when a runner image is first built.
- A Visual Studio developer PowerShell on Windows.
- Docker Desktop WSL integration enabled when running from WSL.

## Run all suites on Windows

Run this command from a Visual Studio developer PowerShell:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\compliance\run-all.ps1
```

Override the target when necessary:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\compliance\run-all.ps1 -DobaHost server.internal -DobaPort 8081
```

`Bypass` applies only to the PowerShell process started by the command; it does
not change the execution policy of the system or the current shell.

## Run all suites on Linux

Run this command from a Bash shell:

```sh
bash compliance/run-all.sh
```

Override the target when necessary:

```sh
DOBA_HOST=server.internal DOBA_PORT=8081 bash compliance/run-all.sh
```

The all-suite runner executes h1spec first and Http11Probe second, even when
h1spec fails. It prints and saves a quantitative summary containing the global
test total and its passed, failed, warning, and error counts. It also includes
the per-suite results, the scored and unscored Http11Probe breakdown, and every
Http11Probe failure. It exits non-zero when either suite cannot run, h1spec
fails, or Http11Probe reports a scored failure or error.

The complete output of each suite is saved under
`compliance/out/all-<timestamp>/`.
The same directory contains `summary.txt` for the whole execution. Http11Probe
also writes its structured report to
`compliance/http11probe/out/http11probe/results.json`. Both output directories
are ignored by Git.

The all-suite runner invokes the two suite-specific runners. Those runners
also create their own `h1spec-<timestamp>/` and
`http11probe-<timestamp>/` directories under `compliance/out/`, each with a
suite-only log and `summary.txt`.

## Run one suite

Use these runners only to investigate one suite in isolation. Each writes its
complete log and a suite-specific `summary.txt` under
`compliance/out/<suite>-<timestamp>/`.

Read `summary.txt` first for the test counts. Use the log for the full suite
output. For Http11Probe, use `results.json` for the structured per-case detail.

### Windows

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\compliance\run-h1spec.ps1
```

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\compliance\run-http11probe.ps1
```

### Linux

```sh
bash compliance/run-h1spec.sh
```

```sh
bash compliance/run-http11probe.sh
```
