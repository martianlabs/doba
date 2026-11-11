<a name="http11probe-compliance-adapter"></a>
<h1>
  <a href="../../../../resources/compliance/http/v11/http11probe/h1-http11probe-compliance-adapter.svg#gh-light-mode-only">
    <img src="../../../../resources/compliance/http/v11/http11probe/h1-http11probe-compliance-adapter.svg" alt="Http11Probe compliance adapter">
  </a>
  <a href="../../../../resources/compliance/http/v11/http11probe/h1-http11probe-compliance-adapter-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/compliance/http/v11/http11probe/h1-http11probe-compliance-adapter-dark.svg" alt="Http11Probe compliance adapter">
  </a>
</h1>

This directory contains the doba adapter and container setup for the official
Http11Probe suite.

<a name="contents"></a>
<h2>
  <a href="../../../../resources/compliance/http/v11/http11probe/h2-contents.svg#gh-light-mode-only">
    <img src="../../../../resources/compliance/http/v11/http11probe/h2-contents.svg" alt="Contents">
  </a>
  <a href="../../../../resources/compliance/http/v11/http11probe/h2-contents-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/compliance/http/v11/http11probe/h2-contents-dark.svg" alt="Contents">
  </a>
</h2>

- `CMakeLists.txt` builds the `doba_http11probe` adapter.
- `main.cpp` exposes the routes required by Http11Probe on port `8080`.
- `Dockerfile` prepares the official Http11Probe suite.
- `docker-compose.yml` defines the suite container, its target, and its report
  volume.

The container setup pins and verifies Http11Probe commit
`d50d818a0e008e9feb3bb9860ad5d9cc40342bc3`.

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
