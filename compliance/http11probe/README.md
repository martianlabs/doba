# Http11Probe compliance adapter

This target exposes the routes Http11Probe uses to probe a HTTP/1.1 server.
It is independent of the doba unit-test and HttpArena benchmark targets.

The checked upstream revision is:

```text
https://github.com/MDA2AV/Http11Probe.git
59513c793ca1f148c82cfedc4b5d75b81d47c591
```

Build and start the adapter:

```text
cmake -S compliance/http11probe -B build/http11probe
cmake --build build/http11probe
build/http11probe/doba_http11probe
```

The adapter implements the probe endpoints `GET /`, `HEAD /`, `OPTIONS /`,
`POST /`, `GET /echo`, `POST /echo`, and `GET /cookie`.

In another terminal, clone the pinned probe revision:

```text
git clone https://github.com/MDA2AV/Http11Probe.git
git -C Http11Probe checkout 59513c793ca1f148c82cfedc4b5d75b81d47c591
```

Run Http11Probe with the .NET 10 SDK container. In PowerShell:

```powershell
$probeRoot = (Resolve-Path Http11Probe).Path
$resultsRoot = Join-Path $PWD "out/compliance/http11probe"
New-Item -ItemType Directory -Force $resultsRoot | Out-Null
docker run --rm `
  -v "${probeRoot}:/probe" `
  -v "${resultsRoot}:/results" `
  -w /probe `
  mcr.microsoft.com/dotnet/sdk:10.0 `
  dotnet run --configuration Release --project src/Http11Probe.Cli -- `
  --host host.docker.internal --port 8080 --output /results/results.json
```

The generated `results.json` is an external test artifact and must not be
committed to this repository.
