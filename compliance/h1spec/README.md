# h1spec compliance adapter

This directory contains the doba adapter and container setup required by the
official h1spec suite. Test orchestration is intentionally kept outside this
repository.

## Contents

- `CMakeLists.txt` builds the `doba_h1spec` adapter.
- `main.cpp` exposes the routes required by h1spec on port `8080`.
- `Dockerfile` prepares the official h1spec suite.
- `docker-compose.yml` defines the suite container and its target.

The container setup pins and verifies h1spec commit
`f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e`.
