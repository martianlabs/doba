# Http11Probe compliance adapter

This directory contains the doba adapter and container setup required by the
official Http11Probe suite. Test orchestration is intentionally kept outside
this repository.

## Contents

- `CMakeLists.txt` builds the `doba_http11probe` adapter.
- `main.cpp` exposes the routes required by Http11Probe on port `8080`.
- `Dockerfile` prepares the official Http11Probe suite.
- `docker-compose.yml` defines the suite container, its target, and its report
  volume.

The container setup pins and verifies Http11Probe commit
`59513c793ca1f148c82cfedc4b5d75b81d47c591`.
