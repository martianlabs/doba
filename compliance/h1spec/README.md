# h1spec compliance adapter

This target exposes the routes h1spec uses to probe a HTTP/1.1 server. It is
independent of the doba unit-test, Http11Probe compliance, and HttpArena
benchmark targets.

## Requirements

- CMake 3.20 or newer and a C++20 compiler.
- Docker Desktop or Docker Engine with Docker Compose V2.
- Network access when the runner image is first built.
- A Visual Studio developer PowerShell on Windows.

The runner downloads h1spec during its image build, checks out commit
`f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e`, and verifies that checkout. No
local clone of h1spec is required.

Use the parent runner to execute the complete compliance battery. This
suite-specific runner configures and builds the adapter, starts it, runs the
suite, and always stops the adapter.

See `compliance/README.md` for the complete battery and its final summary.

## Windows

Run this command from a Visual Studio developer PowerShell:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\compliance\run-h1spec.ps1
```

Override the target when necessary:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\compliance\run-h1spec.ps1 -DobaHost server.internal -DobaPort 8081
```

`Bypass` applies only to the PowerShell process started by the command; it does
not change the execution policy of the system or the current shell.

## Linux

Run this command from a Bash shell:

```sh
bash compliance/run-h1spec.sh
```

Override the target when necessary:

```sh
DOBA_HOST=server.internal DOBA_PORT=8081 bash compliance/run-h1spec.sh
```

`DOBA_HOST` must resolve from inside the runner container. h1spec writes its
complete result to the terminal and returns a non-zero exit status when any
test fails. The runner saves that output to
`compliance/out/h1spec-<timestamp>/h1spec.log` and writes the total, passed,
failed, and warning counts to `summary.txt` in the same directory. h1spec does
not produce a native structured result file.

## Verified result

On 2026-08-14, commit `f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e` passed all
33 h1spec tests against this adapter on Docker Desktop for Windows.
