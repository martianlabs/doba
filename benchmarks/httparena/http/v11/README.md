# HttpArena HTTP/1.1 benchmark adapter

This target exposes the routes HttpArena uses to benchmark a HTTP/1.1 server.
It is independent of the doba unit-test and Http11Probe compliance targets.

The checked upstream revision is:

```text
https://github.com/MDA2AV/HttpArena.git
7bba01a0483fa5ec8ee7e684fe322b98da256150
```

Build and start the adapter locally:

```text
cmake -S benchmarks/httparena/http/v11 -B build/httparena/http/v11
cmake --build build/httparena/http/v11 --config Release
build/httparena/http/v11/doba_httparena_http_v11
```

The adapter implements `GET /baseline11`, `POST /baseline11`,
`GET /pipeline`, and `POST /upload`. Its `meta.json` enables the `baseline`,
`pipelined`, `limited-conn`, and `upload` profiles.

Build and start the submission container:

```text
docker build --tag doba-httparena-http-v11 \
  benchmarks/httparena/http/v11
docker run --rm --publish 8080:8080 doba-httparena-http-v11
```

The Dockerfile accepts `DOBA_REF` as a build argument and defaults to `main`.
Use a published tag or commit when a reproducible benchmark image is required:

```text
docker build --tag doba-httparena-http-v11 \
  --build-arg DOBA_REF=<published-tag-or-commit> \
  benchmarks/httparena/http/v11
```

In another working directory, clone the pinned HttpArena revision:

```text
git clone https://github.com/MDA2AV/HttpArena.git
git -C HttpArena checkout 7bba01a0483fa5ec8ee7e684fe322b98da256150
```

Copy this directory to `HttpArena/frameworks/doba`, then run the declared
profile validations from the HttpArena checkout:

```text
cp -R /path/to/doba/benchmarks/httparena/http/v11 HttpArena/frameworks/doba
cd HttpArena
./scripts/validate.sh doba
```

The HttpArena checkout and any generated results are external benchmark
artifacts and must not be committed to this repository.
