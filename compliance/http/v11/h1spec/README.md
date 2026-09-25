# h1spec compliance adapter

This directory contains the doba adapter and container setup for the official
h1spec suite.

## Contents

- `CMakeLists.txt` builds the `doba_h1spec` adapter.
- `main.cpp` exposes the routes required by h1spec on port `8080`.
- `Dockerfile` prepares the official h1spec suite.
- `docker-compose.yml` defines the suite container and its target.

The container setup pins and verifies h1spec commit
`f0a5650a20c575fbea0f7179a3a9cfa50f20ba6e`.

Build and start the adapter from the doba repository root:

```text
cmake -S compliance/http/v11/h1spec -B build/compliance/http/v11/h1spec
cmake --build build/compliance/http/v11/h1spec --config Release
build/compliance/http/v11/h1spec/doba_h1spec
```

Run all 33 official tests while the adapter listens on port 8080:

```text
docker compose -f compliance/http/v11/h1spec/docker-compose.yml up \
  --build --abort-on-container-exit --exit-code-from h1spec
```

The adapter registers the common methods on `/`. doba suppresses response
bodies for HEAD and treats CONNECT using authority-form request targets.
