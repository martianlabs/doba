# HttpArena HTTP/1.1 benchmark adapter

This target exposes the routes HttpArena uses to benchmark a HTTP/1.1 server.
It is independent of the doba unit-test and Http11Probe compliance targets.

The checked upstream revision is:

```text
https://github.com/MDA2AV/HttpArena.git
484dea628f4fe29de88dc16530b125cc1d169a41
```

Install RapidJSON and zlib development headers, then build and start locally.
The JSON route reads `/data/dataset.json` by default; set `DATASET_PATH` for
a local copy of the official dataset.

```text
cmake -S benchmarks/http/v11/httparena -B build/http/v11/httparena
cmake --build build/http/v11/httparena --config Release
build/http/v11/httparena/doba_httparena_http_v11
```

The adapter implements `GET /baseline11`, `POST /baseline11`,
`GET /json/:count`, and `GET /pipeline`. Its `meta.json` enables the HTTP/1.1
profiles `baseline`, `limited-conn`, `json-comp`, `latency-1m`, `latency-10k`,
and `pipelined`. TLS and `async` remain outside this iteration.

Build and start the submission container:

```text
docker build --tag doba-httparena-http-v11 \
  benchmarks/http/v11/httparena
docker run --rm --publish 8080:8080 doba-httparena-http-v11
```

The Dockerfile accepts `DOBA_REF` as a build argument and defaults to the
published `feature/pipelined` commit used by this adapter. The local
`main.cpp` and the headers selected by `DOBA_REF` must use the same handler
and response-factory API.
Use a published tag or commit when a reproducible benchmark image is required:

```text
docker build --tag doba-httparena-http-v11 \
  --build-arg DOBA_REF=<published-tag-or-commit> \
  benchmarks/http/v11/httparena
```

In another working directory, clone the pinned HttpArena revision:

```text
git clone https://github.com/MDA2AV/HttpArena.git
git -C HttpArena checkout 484dea628f4fe29de88dc16530b125cc1d169a41
```

Copy this directory to `HttpArena/frameworks/doba`, then run the declared
profile validations from the HttpArena checkout:

```text
cp -R /path/to/doba/benchmarks/http/v11/httparena HttpArena/frameworks/doba
cd HttpArena
./scripts/validate.sh doba
```

The HttpArena checkout and any generated results are external benchmark
artifacts and must not be committed to this repository.
