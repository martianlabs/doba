# HttpArena HTTP/1.1 benchmark adapter

This target exposes the routes HttpArena uses to benchmark a HTTP/1.1 server.
It is independent of the doba unit-test and Http11Probe compliance targets.

The checked upstream revision is:

```text
https://github.com/MDA2AV/HttpArena.git
484dea628f4fe29de88dc16530b125cc1d169a41
```

Install RapidJSON, zlib and OpenSSL development headers, then build locally.
The JSON route reads `/data/dataset.json` by default; set `DATASET_PATH` for
a local copy of the official dataset. The TLS listener reads
`/certs/server.crt` and `/certs/server.key`, and the static route reads
`/data/static`.

```text
cmake -S benchmarks/http/v11/httparena -B build/http/v11/httparena
cmake --build build/http/v11/httparena --config Release
```

The adapter listens on port 8080 for HTTP/1.1 and port 8081 for HTTP/1.1 over
TLS. It implements `GET /baseline11`, `POST /baseline11`, `GET /json/:count`,
`GET /pipeline`, `GET /static/*` and `POST /echo`. Its `meta.json` enables
`baseline`, `limited-conn`, `json-comp`, `json-tls`, `latency-1m`,
`latency-10k`, `pipelined`, `8gbit` and `static-tls`.

Build and start the submission container:

```text
docker build --tag doba-httparena-http-v11 \
  benchmarks/http/v11/httparena
docker run --rm --publish 8080:8080 --publish 8081:8081 \
  --volume /path/to/HttpArena/data:/data:ro \
  --volume /path/to/HttpArena/certs:/certs:ro \
  doba-httparena-http-v11
```

The Dockerfile accepts `DOBA_REF` as a build argument and defaults to the
published TLS commit `a5667f843736838b3d66169a91df07ca1fd554a0`.
Use another published tag or commit to test a different doba revision:

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

Copy this directory to `HttpArena/frameworks/doba`, then run the complete
validation of all declared profiles from the HttpArena checkout:

```text
cp -R /path/to/doba/benchmarks/http/v11/httparena HttpArena/frameworks/doba
cd HttpArena
./scripts/validate.sh doba
```

The HttpArena checkout and any generated results are external benchmark
artifacts and must not be committed to this repository.
