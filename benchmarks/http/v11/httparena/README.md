# HttpArena HTTP/1.1 benchmark adapter

This phase measures only **baseline** and **pipelined** while preparing the
Linux transport rewrite. The adapter runs the existing transport unchanged.
It is independent of the doba unit-test and compliance targets; reducing this
driver's profiles does not disable those suites.

## Routes and scope

| Method | Route | Response |
| --- | --- | --- |
| GET | `/baseline11?a=13&b=42` | Query sum: `55`. |
| POST | `/baseline11?a=13&b=42` | Query sum plus integer body; `20` yields `75`. |
| GET | `/pipeline` | Plain text `ok`, including pipelined requests. |

The baseline handlers parse values for each request. POST accepts both
Content-Length and chunked bodies through the engine's decoded body reader.
There is no precomputed baseline response or transport-specific fast path.

Only HTTP on port 8080 is started. There are no TLS, async, JSON, static-file
or echo routes, dataset requirements, certificate mounts or response caches.
`meta.json` declares exactly `baseline` and `pipelined`.

## Build and run

Use a C++20 compiler, CMake 3.20 or newer, and platform thread support.
This target does not require RapidJSON, zlib or OpenSSL.

```sh
cmake -S benchmarks/http/v11/httparena -B out/httparena \
  -DCMAKE_BUILD_TYPE=Release
cmake --build out/httparena --config Release --parallel
WORKERS=1 ./out/httparena/doba_httparena_http_v11
```

The driver defaults to one worker in this phase. `WORKERS` selects a
positive worker count explicitly. Linux uses independent SO_REUSEPORT
listeners, with each connection remaining on its accepting worker.

The container exposes only port 8080 and needs no data volumes:

```sh
docker build --tag doba-httparena-http-v11 \
  benchmarks/http/v11/httparena
docker run --rm --publish 8080:8080 --env WORKERS=1 \
  doba-httparena-http-v11
```

The Dockerfile fetches the library selected by `DOBA_REF`, which defaults
to `main`. To measure the working directory, use the local build above;
the default container build does not include local library changes.

## Local reference against Spliffy

Use the same worker count and server CPU affinity for both servers, and a
separate CPU affinity for the load generator. Run servers sequentially.
Verify response bodies before measuring, including GET, Content-Length POST,
chunked POST, fragmented requests and ordered pipelined responses.

The current single-worker comparison uses these settings:

| Setting | Baseline | Pipelined |
| --- | --- | --- |
| Workers | 1 | 1 |
| Connections | 4096 | 4096 |
| Generator threads | 4 | 4 |
| Pipeline depth | 1 | 16 |
| Warmup on measured sockets | 5 seconds | 5 seconds |
| Measurement | 15 seconds | 30 seconds |
| Repetitions per server | 3 | 3 |

Baseline rotates GET, Content-Length POST and chunked POST raw templates.
Pipelined requests target `/pipeline`. Select the median-throughput run for
each server and profile, keeping latency and resource figures from that run.
Preserve every raw result and alternate server order between repetitions.
Variation up to 10% is expected on this WSL machine.

With the server running on CPU `0`, run the generator from this
directory on CPUs `8,10,12,14`:

```sh
load_generator=/var/tmp/doba-linux-minimal-b3/gcannon-steady/gcannon
taskset -c 8,10,12,14 "$load_generator" http://127.0.0.1:8080 \
  -c 4096 -t 4 -p 1 -d 15s --json \
  --raw requests/get.raw,requests/post_cl.raw,requests/post_chunked.raw
taskset -c 8,10,12,14 "$load_generator" http://127.0.0.1:8080/pipeline \
  -c 4096 -t 4 -p 16 -d 30s --json
```

Block 3 uses a local copy of gcannon 0.6.1. Its observer first waits for
4096 accepted sockets, then releases a five-second warmup on those same
connections before the tool resets counters. Request, parser and statistics
code are unchanged. Use the archived comparison runner for this admission
barrier; the commands above describe the load shape alone.
The original tool remains intact. The patch, binaries and hashes are preserved
under `out/transport-linux/minimal/block-3/`. The runner checks that all 4096
connections are accepted before measuring.

A separate warmup invocation creates different sockets and does not establish
the measured cohort. Do not use it to infer equal-concurrency latency or RSS.
The original transport accepts slowly under load, so historical short runs
can include different active connection counts. Compare current candidates
and the preserved block-2 executable within this new, identical methodology.
These CPU numbers describe this machine; select disjoint physical cores
appropriate to the host when reproducing elsewhere.

Spliffy runs the original baseline and pipeline route implementations, with
other route files omitted from its local runtime and no TLS listener.
Its pipeline handler uses a native declarative response; doba uses its normal
engine and synchronous handler. The comparison includes that difference.

Generated reference artifacts live under
`out/transport-linux/minimal/block-1/`: the original headers and driver,
header hashes, reference executable, comparison script, functional checks,
manifest, raw runs, selected results and report. These are local artifacts,
not published HttpArena leaderboard results. Future blocks must retain this
reference rather than overwrite it.

Linux supports one or more independent SO_REUSEPORT workers.
The new transport supports synchronous callbacks and prefix/source;
TLS and asynchronous routes are outside this phase.

The four-worker block-1 reference remains preserved. Block 2 reruns its
original executable with one worker alongside the candidate and Spliffy;
compare those runs rather than comparing different worker counts.

Block 3 also preserves connection-ramp samples and a memory experiment with
4096 preconnected, fully accepted peers and three checked response rounds.
It separates allocated per-connection memory from slow connection admission;
it is not a replacement for the throughput measurements.

Block 4 compares four workers for each server on physical cores 0,2,4,6;
the generator remains on 8,10,12,14. Both servers use WORKERS=4. All other
load settings and the accepted-connection warmup barrier remain unchanged.
The block-3 single-worker reference remains preserved separately; compare
throughput only with the worker count and CPU affinity stated for each run.
Artifacts are preserved under `out/transport-linux/minimal/block-4/`.

Block 5 repeats the four-worker comparison under the same full-admission
warmup barrier. Final sources, manifests, raw runs and audit evidence live
under `out/transport-linux/minimal/block-5/`.
