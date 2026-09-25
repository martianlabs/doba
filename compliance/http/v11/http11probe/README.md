# Http11Probe compliance adapter

This directory contains the doba adapter and container setup for the official
Http11Probe suite.

## Contents

- `CMakeLists.txt` builds the `doba_http11probe` adapter.
- `main.cpp` exposes the routes required by Http11Probe on port `8080`.
- `Dockerfile` prepares the official Http11Probe suite.
- `docker-compose.yml` defines the suite container, its target, and its report
  volume.

The container setup pins and verifies Http11Probe commit
`c799bdba77e26f85344e181cf6676c0ac1e5ee7a`.

Build and start the adapter from the doba repository root:

```text
cmake -S compliance/http/v11/http11probe \
  -B build/compliance/http/v11/http11probe
cmake --build build/compliance/http/v11/http11probe --config Release
build/compliance/http/v11/http11probe/doba_http11probe
```

Run the full official suite while the adapter listens on port 8080:

```text
docker compose -f compliance/http/v11/http11probe/docker-compose.yml up \
  --build --abort-on-container-exit --exit-code-from http11probe
```

Compose writes the JSON report under
`compliance/http/v11/http11probe/out/http11probe/`. Run from the repository
root so the relative Compose file path resolves. The CLI can exit successfully
when tests fail; inspect the JSON `summary` and individual verdicts.
