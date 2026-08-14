# Http11Probe compliance adapter

This target exposes the routes Http11Probe uses to probe a HTTP/1.1 server. It
is independent of the doba unit-test and HttpArena benchmark targets.

## Requirements

- CMake 3.20 or newer and a C++20 compiler.
- Docker Desktop or Docker Engine with Docker Compose V2.
- Network access when the runner image is first built.
- A Visual Studio developer PowerShell on Windows.

The runner downloads Http11Probe during its image build, checks out commit
`59513c793ca1f148c82cfedc4b5d75b81d47c591`, restores and builds its CLI. No
local clone of Http11Probe is required.

The platform script configures and builds the adapter, starts it, runs the
suite, and always stops the adapter. Run only one compliance suite at a time
because both default to port `8080`.

## Windows

Run this command from a Visual Studio developer PowerShell:

```powershell
.\compliance\http11probe\run.ps1
```

Override the target when necessary:

```powershell
.\compliance\http11probe\run.ps1 -DobaHost server.internal -DobaPort 8081
```

## Linux

Run this command from a Bash shell:

```sh
bash compliance/http11probe/run.sh
```

Override the target when necessary:

```sh
DOBA_HOST=server.internal DOBA_PORT=8081 bash compliance/http11probe/run.sh
```

`DOBA_HOST` must resolve from inside the runner container. Http11Probe writes
`results.json` to `compliance/http11probe/out/http11probe/results.json`. This
directory is ignored by Git; inspect, archive, or remove the artifact after a
run. The CLI can exit with status zero when the report contains failed cases,
so use `summary.failed` and the individual verdicts in `results.json` as the
pass/fail result.

## Verified result

On 2026-08-14, the pinned runner completed 213 probes against this adapter.
The report scored 157/159 tests as passed, with 2 scored failures and 11
warnings. See the generated JSON for the individual cases.
