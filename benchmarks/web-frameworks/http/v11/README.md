# Web Frameworks HTTP/1.1 benchmark adapter

This target exposes the routes used by the Web Frameworks benchmark. It
contains only the doba server adapter; the upstream project owns validation,
load generation, and result collection.

The upstream project inspected for this adapter is:

```text
https://github.com/the-benchmarker/web-frameworks.git
develop (inspected 2026-08-14)
```

Build and start the adapter locally:

```text
cmake -S benchmarks/web-frameworks/http/v11 \
  -B build/web-frameworks/http/v11
cmake --build build/web-frameworks/http/v11 --config Release
build/web-frameworks/http/v11/doba_web_frameworks_http_v11
```

The adapter implements `GET /`, `GET /user/:id`, and `POST /user` on port
3000, with the responses required by the upstream validation suite. The default
upstream profile measures `GET /`, `GET /user/0`, and `POST /user`.

Build and start the standalone adapter container:

```text
docker build --tag doba-web-frameworks-http-v11 \
  benchmarks/web-frameworks/http/v11
docker run --rm --publish 3000:3000 doba-web-frameworks-http-v11
```

The Dockerfile accepts `DOBA_REF` as a build argument and defaults to `main`.
Use a published tag or commit for a reproducible doba image:

```text
docker build --tag doba-web-frameworks-http-v11 \
  --build-arg DOBA_REF=<published-tag-or-commit> \
  benchmarks/web-frameworks/http/v11
```

To use the upstream runner, copy `CMakeLists.txt`, `config.yaml`, and `main.cpp`
to `cpp/doba` in a Web Frameworks checkout. Generate the upstream manifests
and run only the doba adapter:

```text
bundle install
bundle exec rake config
./run.sh cpp/doba
```

Use the same runner for the existing C++ adapters when comparing results:

```text
./run.sh cpp/drogon
./run.sh cpp/oatpp
```

The upstream checkout, generated manifests, and benchmark results are external
artifacts and must not be committed to this repository.
