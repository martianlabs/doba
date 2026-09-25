# Web Frameworks HTTP/1.1 benchmark adapter

This target exposes the routes used by the Web Frameworks benchmark. It
contains only the doba server adapter; the upstream project owns validation,
load generation, and result collection.

The upstream project inspected for this adapter is:

```text
https://github.com/the-benchmarker/web-frameworks.git
50cb0223f095878106e3115ed06261056870032b
```

Build and start the adapter locally:

```text
cmake -S benchmarks/web-frameworks/http/v11 \
  -B build/web-frameworks/http/v11
cmake --build build/web-frameworks/http/v11 --config Release
build/web-frameworks/http/v11/doba_web_frameworks_http_v11
```

The adapter implements `GET /`, `GET /user/:id`, and `POST /user` on port
3000. The user route returns the path parameter; the other routes return an
empty body. The upstream route specification checks all three routes. The
default benchmark measures only `GET /`.

Build and start the standalone adapter container:

```text
docker build --tag doba-web-frameworks-http-v11 \
  benchmarks/web-frameworks/http/v11
docker run --rm --publish 3000:3000 doba-web-frameworks-http-v11
```

The Dockerfile accepts `DOBA_REF` as a build argument and defaults to the
published `feature/pipelined` commit used by the upstream configuration.
Use another published tag or commit to test a different doba revision:

```text
docker build --tag doba-web-frameworks-http-v11 \
  --build-arg DOBA_REF=<published-tag-or-commit> \
  benchmarks/web-frameworks/http/v11
```

To use the upstream runner, copy `CMakeLists.txt`, `config.yaml`, and `main.cpp`
to `cpp/doba` in a checkout of the pinned revision. Install Ruby, Bundler,
Docker, `jq`, and `zrk` 2.4 or newer. Generate the upstream manifests for
the three routes and concurrency levels:

```text
bundle install
CONCURRENCIES=64,256,512 \
ROUTES='GET:/,GET:/user/0,POST:/user' \
SERVER_CPUS=0-3 LOAD_CPUS=4-15 bundle exec rake config
```

Run the generated targets for each adapter under the same conditions:

```text
make -f cpp/doba/.Makefile build
make -f cpp/doba/.Makefile test
make -f cpp/doba/.Makefile warmup
mkdir -p cpp/doba/.results/{64,256,512}
make -f cpp/doba/.Makefile collect
make -f cpp/doba/.Makefile unbuild
```

Apply the same targets to `cpp/drogon` and `cpp/oatpp`. The `collect` target
uses `zrk --closed` for 15 seconds per route by default and records request
rate, latency, and server CPU saturation. The upstream checkout, generated
manifests, and benchmark results are external artifacts and must not be
committed to this repository.
