<a name="httparena-http11-benchmark-adapter"></a>
<h1>
  <a href="../../../../resources/benchmarks/http/v11/httparena/h1-httparena-http11-benchmark-adapter.svg#gh-light-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/httparena/h1-httparena-http11-benchmark-adapter.svg" alt="HttpArena">
  </a>
  <a href="../../../../resources/benchmarks/http/v11/httparena/h1-httparena-http11-benchmark-adapter-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/httparena/h1-httparena-http11-benchmark-adapter-dark.svg" alt="HttpArena">
  </a>
</h1>

One driver, eleven HTTP/1.1 profiles: **baseline**, **pipelined**, **limited-conn**,
**latency-10k**, **latency-1m**, **latency-500k-8cpu**, **async**, **json-comp**,
**json-tls**, **8gbit** and **static-tls**.

Plain HTTP listens on 8080; TLS listens on 8081. The delay handler is a coroutine.
The others use ordinary synchronous handlers.

<a name="routes-and-scope"></a>
<h2>
  <a href="../../../../resources/benchmarks/http/v11/httparena/h2-routes-and-scope.svg#gh-light-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/httparena/h2-routes-and-scope.svg" alt="What it serves">
  </a>
  <a href="../../../../resources/benchmarks/http/v11/httparena/h2-routes-and-scope-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/httparena/h2-routes-and-scope-dark.svg" alt="What it serves">
  </a>
</h2>

| Method | Route | Reply |
| --- | --- | --- |
| GET | `/baseline11?a=13&b=42` | The query sum: `55`. |
| POST | `/baseline11?a=13&b=42` | Query sum plus the decoded integer body. |
| GET | `/pipeline` | Plain text `ok`. |
| GET | `/delay/:ms` | Wait that many milliseconds, then echo the value. |
| GET | `/json/:count?m=...` | Copy N dataset objects, calculate totals, serialize JSON and negotiate gzip. TCP and TLS. |
| POST | `/echo` (TLS) | Echo the decoded body through doba's reader. |
| GET/HEAD | `/static/*` (TLS) | Built-in file server, shared cache and precompressed files. |

The baseline parses every request. Bad integers and overflowing sums return
400; POST accepts Content-Length and chunked bodies. Nothing is precomputed.

The delay comes from the route parameter: a whole integer from 0 to 60000.
Zero stays zero; bad values return 400. Each request owns its wait and uses
`common::sleep_for`, so it doesn't block the worker. Sync and async replies
can share a pipeline and still come out in order.
See the upstream [implementation](https://http-arena.com/docs/test-profiles/h1/async/implementation/)
and [validation](https://http-arena.com/docs/test-profiles/h1/async/validation/) notes.

JSON loads the dataset once, but copies, totals, serialization and gzip happen
per request. Other source fields stay intact. Compression honors
Accept-Encoding and sends Vary: Accept-Encoding. Static files use the library's
cache; the handler doesn't keep a second one.

Workers are picked automatically from hardware concurrency, Linux CPU affinity
and cgroup v1/v2 quotas, with at least one worker. Linux uses independent
SO_REUSEPORT listeners, and each connection stays with its accepting worker.
Memory and buffering policies keep doba's defaults.

| Optional override | Default |
| --- | --- |
| `WORKERS` | Automatic; an override must be positive. |
| `PORT`, `TLS_PORT` | 8080, 8081. |
| `DATASET_PATH` | `/data/dataset.json`. |
| `STATIC_DIR` | `/data/static`. |
| `TLS_CERT`, `TLS_KEY` | `/certs/server.crt`, `/certs/server.key`. |

The full driver starts both listeners. Have the data and certificates ready;
missing inputs fail startup.

<a name="build-and-run"></a>
<h2>
  <a href="../../../../resources/benchmarks/http/v11/httparena/h2-build-and-run.svg#gh-light-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/httparena/h2-build-and-run.svg" alt="Build and run">
  </a>
  <a href="../../../../resources/benchmarks/http/v11/httparena/h2-build-and-run-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/httparena/h2-build-and-run-dark.svg" alt="Build and run">
  </a>
</h2>

You'll need C++20, CMake 3.20+, Ninja, OpenSSL 3, zlib and RapidJSON.
From the repo root, in a Unix shell:

```sh
cmake -S benchmarks/http/v11/httparena -B out/httparena -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build out/httparena --parallel
./out/httparena/doba_httparena_http_v11
```

Standalone builds default to `DOBA_HTTPARENA_FULL=ON`. Root test builds
default to TCP-only so ordinary CI doesn't need benchmark JSON/TLS dependencies.
Set that flag to ON there to check the full adapter too.

`doba_httparena_tests` checks real replies, arithmetic errors, fragmented and
chunked bodies, sixteen-response pipelines, zero/different overlapping delays
and mixed reply order. Linux also checks workers under restricted affinity.
These are correctness checks. Use HttpArena's runner and `validate.sh` for
its full validation and timed workloads.

For Docker, put the data and certificates under `./data` and `./certs`:

```sh
docker build --build-arg DOBA_REF=main --tag doba-httparena-http-v11 benchmarks/http/v11/httparena
docker run --rm --publish 8080:8080 --publish 8081:8081 \
  --mount type=bind,src="$PWD/data",dst=/data,readonly \
  --mount type=bind,src="$PWD/certs",dst=/certs,readonly doba-httparena-http-v11
```

Docker fetches `DOBA_REF` from GitHub, defaulting to main. Pick the matching
published revision; it won't include your uncommitted library changes.
Use the local build above to try those.

<a name="historical-local-reference-against-spliffy"></a>
<h2>
  <a href="../../../../resources/benchmarks/http/v11/httparena/h2-historical-local-reference-against-spliffy.svg#gh-light-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/httparena/h2-historical-local-reference-against-spliffy.svg" alt="Older local runs">
  </a>
  <a href="../../../../resources/benchmarks/http/v11/httparena/h2-historical-local-reference-against-spliffy-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/benchmarks/http/v11/httparena/h2-historical-local-reference-against-spliffy-dark.svg" alt="Older local runs">
  </a>
</h2>

The `out/async-processing/block-2/` archive records a paired comparison with
Spliffy 1.7.0. These are local measurements, not HttpArena leaderboard scores.
The saved runners may expect the earlier three-profile driver; they aren't the
current CI tools. Use their saved dependencies and manifests when reproducing them.

| Setting | Linux / WSL | Windows |
| --- | --- | --- |
| Workers per server | 4 | 1 |
| Server CPUs | 0,2,4,6 | 0 |
| Connections | 4096 | 1024 |
| Generator threads | 4 | 8 |
| Generator CPUs | 8,10,12,14 | 4-15 |
| Observer CPU | 15 | 2 |
| Generator | gcannon 0.6.1 | autocannon 8.0.0 |
| Node for Spliffy | 26.13.0 | 24.16.0 |

Each run warms the admitted sockets for 5 seconds. Baseline then measures
10 seconds at depth one; pipelined measures 15 seconds at depth sixteen, in
batches rather than a sliding window. Baseline rotates GET, Content-Length
POST and chunked POST; pipelined uses `/pipeline`.

Both servers get the same workers and CPU affinity, with separate generator
cores, and run in turn. Validate bodies before measuring. Failed admission,
socket errors, timeouts, reconnects or bad replies reject a trial. Windows
also rejects warmup errors; Linux resets those counters after warmup.

Run three alternating repetitions. Keep the median-throughput run's own
latency, CPU and memory numbers, plus the spread across runs. CPU/peak RSS
include warmup; throughput/latency don't. Summed RSS can count shared pages
twice, and different platforms' resource numbers aren't directly comparable.
The old notes warn of up to 10% variation on that WSL setup.

The archive keeps patched generators, templates, original files, binaries and
raw output, and refuses to overwrite results. Spliffy uses its original
baseline/pipeline handlers, including a declarative pipeline reply; doba uses
its normal parser and handler. That difference is part of the comparison.
The saved pre-async `original` build uses the same compiler and Release flags.

The older transport-rewrite runs in `out/transport-linux/minimal/` are a
separate comparison. Keep their numbers separate too.
