# h1spec compliance adapter

This target exposes the routes h1spec uses to probe a HTTP/1.1 server.
It is independent of the doba unit-test, Http11Probe compliance, and HttpArena
benchmark targets.

Build and start the adapter:

```text
cmake -S compliance/h1spec -B build/h1spec
cmake --build build/h1spec
build/h1spec/doba_h1spec
```

The adapter implements `GET /` and `POST /`. Both routes echo the decoded
request body as required by the current h1spec suite.

Run the official h1spec suite with Docker/Deno. The first command resolves the
exact h1spec revision to test; the second runs that revision. In PowerShell:

```powershell
$h1specSha = (docker run --rm alpine/git `
  ls-remote https://github.com/uNetworking/h1spec.git HEAD).Split()[0]
docker run --rm `
  denoland/deno:alpine `
  run --allow-net `
  "https://raw.githubusercontent.com/uNetworking/h1spec/$h1specSha/http_test.ts" `
  host.docker.internal 8080
```

h1spec does not generate a result artifact. Its process exits with a non-zero
status when any test fails.

## Verified result

On 2026-08-14, commit `f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e` passed all
33 h1spec tests against this adapter on Docker Desktop for Windows.
