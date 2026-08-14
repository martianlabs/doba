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

The platform script configures and builds the adapter, starts it, runs the
suite, and always stops the adapter. Run only one compliance suite at a time
because both default to port `8080`.

## Windows

Run this command from a Visual Studio developer PowerShell:

```powershell
.\compliance\h1spec\run.ps1
```

Override the target when necessary:

```powershell
.\compliance\h1spec\run.ps1 -DobaHost server.internal -DobaPort 8081
```

## Linux

Run this command from a Bash shell:

```sh
bash compliance/h1spec/run.sh
```

Override the target when necessary:

```sh
DOBA_HOST=server.internal DOBA_PORT=8081 bash compliance/h1spec/run.sh
```

`DOBA_HOST` must resolve from inside the runner container. h1spec writes its
complete result to the terminal and returns a non-zero exit status when any
test fails; it does not create a result artifact.

## Verified result

On 2026-08-14, commit `f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e` passed all
33 h1spec tests against this adapter on Docker Desktop for Windows.
