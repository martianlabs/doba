<a name="benchmarks"></a>
<h1>
  <a href="../../resources/benchmarks/overview/h1-benchmarks.svg#gh-light-mode-only">
    <img src="../../resources/benchmarks/overview/h1-benchmarks.svg" alt="Benchmarks">
  </a>
  <a href="../../resources/benchmarks/overview/h1-benchmarks-dark.svg#gh-dark-mode-only">
    <img src="../../resources/benchmarks/overview/h1-benchmarks-dark.svg" alt="Benchmarks">
  </a>
</h1>

[Back to doba](../../README.md#okay-how-fast)

**The current run, in full.** The README shows the highlights. Here are the
ranking, every measured workload, and the context behind the numbers.

[Overall](#overall) / [Baseline](#baseline) / [Pipelined](#pipelined) /
[Limited connections](#limited-conn) / [Latency 10k](#latency-10k) /
[Latency 1m](#latency-1m) / [Latency 500k, 8 CPUs](#latency-500k-8cpu) /
[Async](#async) / [JSON compression](#json-comp) / [JSON TLS](#json-tls) /
[8 Gbit](#8gbit) / [Static TLS](#static-tls) / [RAM](#memory)

Big throughput numbers are fun. Waiting for a response isn't. Let's look at both.

Measured on **Scaleway, 2026-10-28**, using the official HttpArena suite:
native **gcannon** and **wrk**, with **zrk** and servers in Docker, following
upstream CPU affinity and host tuning. **RUNS=3**. Results are selected by
upstream from its repetitions, not averaged across runs.
The campaign's reference-model verification is **incomplete**; these are
local results, not the published HttpArena leaderboard.

Every chart shows the **same top 15 from the overall ranking**, ordered by each
test's metric. Scores retain normalization across all **21 measured servers**.
The total includes eight scored profiles and the upstream completeness factor;
**pipelined**, **latency-500k-8cpu** and **static-tls** are reference-only profiles.
Failed and skipped measurements are shown explicitly; they are not measured zeroes.

The test machine uses an **AMD EPYC 8534P at 2.30 GHz**
(**64 cores / 128 threads**), **576 GiB of DDR5 RAM** and
**2 x 3.84 TB NVMe** storage. It provides a **25 Gbps private network**
and a **1 Gbps public connection**, upgradable to **25 Gbps**.

<a name="overall"></a>
<a href="../../resources/benchmarks/benchmark-overall.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-overall.svg" alt="HttpArena overall top 15: total points including completeness; higher is better." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-overall.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-overall-dark.svg" alt="HttpArena overall top 15: total points including completeness; higher is better." width="100%">
</a>
<br><br>

<a name="baseline"></a>
<a href="../../resources/benchmarks/benchmark-baseline.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-baseline.svg" alt="HttpArena baseline: selected requests per second; higher is better." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-baseline.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-baseline-dark.svg" alt="HttpArena baseline: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a name="pipelined"></a>
<a href="../../resources/benchmarks/benchmark-pipelined.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-pipelined.svg" alt="HttpArena pipelined: selected requests per second; higher is better." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-pipelined.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-pipelined-dark.svg" alt="HttpArena pipelined: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a name="limited-conn"></a>
<a href="../../resources/benchmarks/benchmark-limited-conn.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-limited-conn.svg" alt="HttpArena limited-conn: selected requests per second; higher is better." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-limited-conn.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-limited-conn-dark.svg" alt="HttpArena limited-conn: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a name="latency-10k"></a>
<a href="../../resources/benchmarks/benchmark-latency-10k.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-latency-10k.svg" alt="HttpArena latency-10k: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-latency-10k.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-latency-10k-dark.svg" alt="HttpArena latency-10k: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<br><br>

<a name="latency-1m"></a>
<a href="../../resources/benchmarks/benchmark-latency-1m.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-latency-1m.svg" alt="HttpArena latency-1m: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-latency-1m.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-latency-1m-dark.svg" alt="HttpArena latency-1m: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<br><br>

<a name="latency-500k-8cpu"></a>
<a href="../../resources/benchmarks/benchmark-latency-500k-8cpu.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-latency-500k-8cpu.svg" alt="HttpArena latency-500k-8cpu: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-latency-500k-8cpu.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-latency-500k-8cpu-dark.svg" alt="HttpArena latency-500k-8cpu: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<br><br>

<a name="async"></a>
<a href="../../resources/benchmarks/benchmark-async.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-async.svg" alt="HttpArena async: selected requests per second; higher is better." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-async.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-async-dark.svg" alt="HttpArena async: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a name="json-comp"></a>
<a href="../../resources/benchmarks/benchmark-json-comp.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-json-comp.svg" alt="HttpArena json-comp: compression-adjusted requests per second; higher is better." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-json-comp.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-json-comp-dark.svg" alt="HttpArena json-comp: compression-adjusted requests per second; higher is better." width="100%">
</a>
<br><br>

<a name="json-tls"></a>
<a href="../../resources/benchmarks/benchmark-json-tls.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-json-tls.svg" alt="HttpArena json-tls: selected requests per second; higher is better." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-json-tls.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-json-tls-dark.svg" alt="HttpArena json-tls: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a name="8gbit"></a>
<a href="../../resources/benchmarks/benchmark-8gbit.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-8gbit.svg" alt="HttpArena 8gbit: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-8gbit.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-8gbit-dark.svg" alt="HttpArena 8gbit: upstream fixed-rate score, p99 latency, achieved rate and status." width="100%">
</a>
<br><br>

<a name="static-tls"></a>
<a href="../../resources/benchmarks/benchmark-static-tls.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-static-tls.svg" alt="HttpArena static-tls: selected requests per second; higher is better." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-static-tls.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-static-tls-dark.svg" alt="HttpArena static-tls: selected requests per second; higher is better." width="100%">
</a>
<br><br>

<a name="memory"></a>

**What about RAM?** These charts show the **sampled peak of container memory**
from each upstream-selected repetition. They use the same overall top 15,
with **solid horizontal bars on a linear scale from zero** and the original
values and units. Each workload uses its own axis range.
Lower is better; memory alone does not measure throughput efficiency.

Rows run from lowest to highest RAM. JSON compression has separate panels
for **4,096 and 16,384 connections**, both ordered by the 4,096-connection
measurement. Both panels share the same scale; their memory values are not
averaged.

The original selected-result JSON also records RAM for the latency and
8 Gbit tests, although their console tables omit it. Those charts include
**achieved rate and status**: rate-penalized runs handled less traffic.
All source values are preserved in the complete parsed data linked below.

[Baseline](#memory-baseline) / [Pipelined](#memory-pipelined) / [Limited connections](#memory-limited-conn) / [Latency 10k](#memory-latency-10k) / [Latency 1m](#memory-latency-1m) / [Latency 500k, 8 CPUs](#memory-latency-500k-8cpu) / [Async](#memory-async) / [JSON compression](#memory-json-comp) / [JSON TLS](#memory-json-tls) / [8 Gbit](#memory-8gbit) / [Static TLS](#memory-static-tls)

<a name="memory-baseline"></a>
<a href="../../resources/benchmarks/benchmark-memory-baseline.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-baseline.svg" alt="HttpArena baseline: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-memory-baseline.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-baseline-dark.svg" alt="HttpArena baseline: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<br><br>

<a name="memory-pipelined"></a>
<a href="../../resources/benchmarks/benchmark-memory-pipelined.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-pipelined.svg" alt="HttpArena pipelined: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-memory-pipelined.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-pipelined-dark.svg" alt="HttpArena pipelined: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<br><br>

<a name="memory-limited-conn"></a>
<a href="../../resources/benchmarks/benchmark-memory-limited-conn.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-limited-conn.svg" alt="HttpArena limited-conn: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-memory-limited-conn.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-limited-conn-dark.svg" alt="HttpArena limited-conn: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<br><br>

<a name="memory-latency-10k"></a>
<a href="../../resources/benchmarks/benchmark-memory-latency-10k.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-latency-10k.svg" alt="HttpArena latency-10k: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels. Achieved rate and measurement status are shown alongside RAM." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-memory-latency-10k.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-latency-10k-dark.svg" alt="HttpArena latency-10k: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels. Achieved rate and measurement status are shown alongside RAM." width="100%">
</a>
<br><br>

<a name="memory-latency-1m"></a>
<a href="../../resources/benchmarks/benchmark-memory-latency-1m.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-latency-1m.svg" alt="HttpArena latency-1m: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels. Achieved rate and measurement status are shown alongside RAM." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-memory-latency-1m.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-latency-1m-dark.svg" alt="HttpArena latency-1m: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels. Achieved rate and measurement status are shown alongside RAM." width="100%">
</a>
<br><br>

<a name="memory-latency-500k-8cpu"></a>
<a href="../../resources/benchmarks/benchmark-memory-latency-500k-8cpu.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-latency-500k-8cpu.svg" alt="HttpArena latency-500k-8cpu: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels. Achieved rate and measurement status are shown alongside RAM." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-memory-latency-500k-8cpu.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-latency-500k-8cpu-dark.svg" alt="HttpArena latency-500k-8cpu: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels. Achieved rate and measurement status are shown alongside RAM." width="100%">
</a>
<br><br>

<a name="memory-async"></a>
<a href="../../resources/benchmarks/benchmark-memory-async.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-async.svg" alt="HttpArena async: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-memory-async.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-async-dark.svg" alt="HttpArena async: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<br><br>

<a name="memory-json-comp"></a>
<a href="../../resources/benchmarks/benchmark-memory-json-comp.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-json-comp.svg" alt="HttpArena json-comp: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels. Separate 4,096 and 16,384 connection panels, ordered by RAM at 4,096 connections." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-memory-json-comp.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-json-comp-dark.svg" alt="HttpArena json-comp: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels. Separate 4,096 and 16,384 connection panels, ordered by RAM at 4,096 connections." width="100%">
</a>
<br><br>

<a name="memory-json-tls"></a>
<a href="../../resources/benchmarks/benchmark-memory-json-tls.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-json-tls.svg" alt="HttpArena json-tls: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-memory-json-tls.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-json-tls-dark.svg" alt="HttpArena json-tls: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<br><br>

<a name="memory-8gbit"></a>
<a href="../../resources/benchmarks/benchmark-memory-8gbit.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-8gbit.svg" alt="HttpArena 8gbit: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels. Achieved rate and measurement status are shown alongside RAM." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-memory-8gbit.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-8gbit-dark.svg" alt="HttpArena 8gbit: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels. Achieved rate and measurement status are shown alongside RAM." width="100%">
</a>
<br><br>

<a name="memory-static-tls"></a>
<a href="../../resources/benchmarks/benchmark-memory-static-tls.svg#gh-light-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-static-tls.svg" alt="HttpArena static-tls: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<a href="../../resources/benchmarks/benchmark-memory-static-tls.svg#gh-dark-mode-only" title="Open full-size light chart">
  <img src="../../resources/benchmarks/benchmark-memory-static-tls-dark.svg" alt="HttpArena static-tls: sampled peak memory for the overall top 15; lower is better; linear scale from zero; independent scale per workload, shared by its connection panels." width="100%">
</a>
<br><br>

**Source and measurement notes:** [Original summary](../../resources/benchmarks/summary.txt)
and [complete parsed data](../../resources/benchmarks/data.json).
These include every measured server, revisions, failures, skipped profiles
and measurement warnings. Only the current dataset is retained here.
