# HttpArena HTTP/1.1 benchmark adapter

This target exposes the routes HttpArena uses to benchmark a HTTP/1.1 server.
It is independent of the doba unit-test and Http11Probe compliance targets.

The checked upstream revision is:

```text
https://github.com/MDA2AV/HttpArena.git
5b111b8889fee61076f1d7dd14fdb3dfd12c8492
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

The gzip handler reuses one zlib context per worker thread and resets it
for each response. JSON and compressed bytes are generated for every request;
there is no response cache. The context releases its memory when the worker
thread exits. Keeping these allocations trades retained per-thread memory
for avoiding allocation and release on every response. Compression parameters
remain unchanged.

The TLS `/static/*` GET/HEAD routes are registered by Doba's documented
`static_file_server` controller. Its built-in cache is enabled with default
cache policies, and `precompressed = true` selects the on-disk Brotli/gzip
representations. There is no benchmark-specific file loader, MIME table,
preloaded directory or response-body cache in the adapter. HTTP and transport
memory policies remain at their library defaults.

Cached hits now validate identity, size, modification/change times, flags and link
count through the library's confined attribute query. A changed snapshot is
invalidated even for equal bytes. TTL limits retention; it does not delay file
validation. Missing compressed variants are checked without a read open and are
not cached, so publication is not hidden by a negative-cache window. There is no
native monitor. Future timestamps, immutable readers and HTTP validators remain
handled by the controller. Build against a Doba revision containing these changes;
changing this adapter alone does not update the library fetched by Docker.

The Fulmine review informed two adapter choices: on-disk compressed variants stay
enabled and worker count follows CPU affinity and cgroup quota (at least one,
rounded down for fractional cores). Cache sharing differs deliberately: Doba uses
one bounded library cache across workers, rather than multiplying its memory
budget per worker/process. Concurrency improvements and Last-Modified reuse are
library capabilities, not a benchmark-specific fast path. The driver does not
copy Trillium's one-second response-freshness window or Fulmine's negative TTL.

For local and container reproduction, STATIC_DIR overrides /data/static; TLS_CERT
and TLS_KEY override the two /certs paths. Defaults preserve the official mounts.
Startup reports effective workers, asset root, shared entry/cost limits and the
maximum cacheable file size. Run the official static-tls profile to measure the
twenty-route workload with this adapter.

The adapter listens on port 8080 for HTTP/1.1 and port 8081 for HTTP/1.1 over
TLS. It implements `GET /baseline11`, `POST /baseline11`, `GET /delay/:ms`,
`GET /json/:count`, `GET /static/*` and `POST /echo`. The delay route suspends
until the requested milliseconds elapse, then returns the number as plain text.
It resumes on the transport I/O worker without a driver timer thread.
Its `meta.json` enables `baseline`, `limited-conn`, `async`, `json-comp`,
`json-tls`, `latency-1m`, `latency-10k`, `latency-500k-8cpu`, `8gbit` and
`static-tls`.

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
`main` branch.
Use another published tag or commit to test a different doba revision:

```text
docker build --tag doba-httparena-http-v11 \
  --build-arg DOBA_REF=<published-tag-or-commit> \
  benchmarks/http/v11/httparena
```

In another working directory, clone the pinned HttpArena revision:

```text
git clone https://github.com/MDA2AV/HttpArena.git
git -C HttpArena checkout 5b111b8889fee61076f1d7dd14fdb3dfd12c8492
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
