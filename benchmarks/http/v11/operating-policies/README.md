# Operating policy overhead

This microbenchmark isolates HTTP engine CPU cost with finite default policies.
It runs one million requests per sample, five samples for each route: ordinary
synchronous handling and an async handler that completes immediately. The transport
pressure callback models a published flag by default; it can model the previous
weak-reference and mutex query for comparison. Output is consumed immediately.
An idle auxiliary thread keeps standard-library reference counting and locks on
the multithreaded paths used by the real server. It does not measure sockets, admission, inactivity scanning, TLS or HttpArena RPS.

Build on Linux without sanitizers or concurrent compilation:

```sh
cmake -S benchmarks/http/v11/operating-policies -B out/build/policy-perf   -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++
cmake --build out/build/policy-perf
taskset -c 2 out/build/policy-perf/operating_policies_benchmark
```

Each line reports the route, CPU nanoseconds per request, output bytes, pressure
queries, ready callbacks and resume calls. Compare medians across multiple runs
with the same compiler and affinity. On Linux `std::clock()` measures process CPU
time; these timings are not end-to-end latency or wall-clock throughput.

To compare earlier headers, export the desired revision's `include/` with
`git archive` and set `-DDOBA_INCLUDE_DIR=/absolute/path/to/include` in a separate
build directory. For the original `feature/limits` headers, also set
`-DDOBA_BENCH_SKIP_UNPAUSED_RESUME=OFF` and
`-DDOBA_BENCH_PUBLISHED_PRESSURE=OFF` to model its unconditional Linux resume
calls and its pressure-query locking. Older engines without flow hooks compile as well. Use the same benchmark
source for every revision and alternate their execution order.

Keep the real TCP/TLS integration tests and the HttpArena comparison alongside
this measurement: a cheaper engine loop alone does not establish overall server
performance or verify backpressure correctness.

The same CMake project also builds `operating_activity_benchmark`. It compares
context construction, positive I/O markers and maintenance at 50, 100, 250, 500
and 1000 ms with 4096 or 32000 contexts. Each sample models one million requests
per second, two completion boundaries per request, and the existing context
locks. Five samples report CPU time, not socket throughput. The logical sweep
schedule is simulated; no thread sleeps or real network load are included.
Use the same source with the previous headers to compare per-I/O timestamps
against sampled activity, and verify the result separately with TCP/TLS traffic.
