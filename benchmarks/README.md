# Benchmark runner

This Docker Compose stack runs the Web Frameworks and HttpArena suites without
requiring Ruby, Oha, Bash, Git, or a Docker socket on the host. The `daemon`
service is an isolated Docker daemon. The `runner` service contains every
tool required by the upstream projects and controls that daemon over the
network namespace. This lets Oha reach the private bridge IPs assigned to the
benchmark servers created by the inner daemon.

The two benchmark adapters remain read-only mounts:

- `benchmarks/web-frameworks/http/v11`
- `benchmarks/httparena/http/v11`

## Requirements

- Docker Desktop or Docker Engine with Docker Compose V2.
- Network access when the runner or an upstream image is first built.
- Docker Desktop WSL integration enabled when running from WSL.

## Run all suites

The following examples run from the repository root. The Windows commands
bypass the script policy for their process only:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\benchmarks\run-all.ps1
```

On Linux:

```sh
bash benchmarks/run-all.sh
```

Both commands benchmark the default framework subsets. To validate them
without collecting benchmark measurements, use:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\benchmarks\run-all.ps1 -Mode validate
```

```sh
BENCHMARK_MODE=validate bash benchmarks/run-all.sh
```

The all-suite runners execute Web Frameworks first and HttpArena second, even
when Web Frameworks fails. They exit non-zero when either suite fails.

## Select frameworks

The defaults are defined in `targets.env`. Override both subsets on Windows
with:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\benchmarks\run-all.ps1 -WebFrameworks "doba,drogon,oatpp" -HttpArenaFrameworks "doba,web-framework-cpp"
```

On Linux:

```sh
WEB_FRAMEWORKS=doba,drogon,oatpp HTTPARENA_FRAMEWORKS=doba,web-framework-cpp bash benchmarks/run-all.sh
```

The scripts rebuild the runner automatically, so changes to `targets.env`
take effect on the next execution. The remaining framework names must exist in
the corresponding upstream project.

## Run one suite

Use the individual runners to execute one suite. On Windows:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\benchmarks\run-web-frameworks.ps1
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\benchmarks\run-httparena.ps1
```

Select frameworks or validation mode with:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\benchmarks\run-web-frameworks.ps1 -Mode validate -Frameworks "doba"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\benchmarks\run-httparena.ps1 -Mode validate -Frameworks "doba"
```

On Linux:

```sh
bash benchmarks/run-web-frameworks.sh
bash benchmarks/run-httparena.sh
```

Select frameworks or validation mode with:

```sh
BENCHMARK_MODE=validate WEB_FRAMEWORKS=doba bash benchmarks/run-web-frameworks.sh
BENCHMARK_MODE=validate HTTPARENA_FRAMEWORKS=doba bash benchmarks/run-httparena.sh
```

## Results

The runner creates temporary upstream checkouts in the `runner-work` volume.
Exported logs and results are written under `artifacts/`, which is ignored by
Git. Each wrapper stops the Compose stack after execution while preserving
these artifacts and the named cache volumes.

## Implementation

The platform wrappers call Docker Compose and build the runner automatically.
`benchmark-runner.sh` is the container's internal entry point and is not
invoked directly from the host. It copies the mounted `doba` adapter into
each temporary upstream checkout before running the official workflows.

The daemon is isolated from Docker Desktop's image cache and containers. Remove
its cached checkouts and inner images only when they are no longer needed:

```powershell
docker compose -f benchmarks/compose.yaml down --volumes
```
