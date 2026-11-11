<a name="benchmarks"></a>
<h1>
  <a href="../resources/benchmarks/overview/h1-benchmarks.svg#gh-light-mode-only">
    <img src="../resources/benchmarks/overview/h1-benchmarks.svg" alt="Benchmarks">
  </a>
  <a href="../resources/benchmarks/overview/h1-benchmarks-dark.svg#gh-dark-mode-only">
    <img src="../resources/benchmarks/overview/h1-benchmarks-dark.svg" alt="Benchmarks">
  </a>
</h1>

How fast does it go? These adapters let external benchmark runners find out.
doba serves the requests; the upstream tools generate traffic and collect results.

<a name="httparena"></a>
<h2>
  <a href="../resources/benchmarks/overview/h2-httparena.svg#gh-light-mode-only">
    <img src="../resources/benchmarks/overview/h2-httparena.svg" alt="HttpArena">
  </a>
  <a href="../resources/benchmarks/overview/h2-httparena-dark.svg#gh-dark-mode-only">
    <img src="../resources/benchmarks/overview/h2-httparena-dark.svg" alt="HttpArena">
  </a>
</h2>

The full adapter handles eleven HTTP/1.1 profiles: simple replies, pipelines,
delayed handlers, JSON, gzip, TLS and static files.
[Build and run it](http/v11/httparena/README.md).

Standalone and Docker builds include everything. Root test builds use a smaller
TCP-only version unless you set `DOBA_HTTPARENA_FULL=ON`.
The driver has correctness tests too; a passing test isn't a throughput score.

<a name="web-frameworks"></a>
<h2>
  <a href="../resources/benchmarks/overview/h2-web-frameworks.svg#gh-light-mode-only">
    <img src="../resources/benchmarks/overview/h2-web-frameworks.svg" alt="Web Frameworks">
  </a>
  <a href="../resources/benchmarks/overview/h2-web-frameworks-dark.svg#gh-dark-mode-only">
    <img src="../resources/benchmarks/overview/h2-web-frameworks-dark.svg" alt="Web Frameworks">
  </a>
</h2>

Three small routes: a root GET, a user GET and a user POST.
[Use the adapter](http/v11/web-frameworks/README.md) on its own or with the
Web Frameworks runner.

For current transport comparisons, use HttpArena. Older activity/flow-control
simulations don't measure today's engine.
