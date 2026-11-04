<a name="benchmarks"></a>
<h1>
  <a href="../resources/benchmarks/overview/h1-benchmarks.svg#gh-light-mode-only">
    <img src="../resources/benchmarks/overview/h1-benchmarks.svg" alt="Benchmarks">
  </a>
  <a href="../resources/benchmarks/overview/h1-benchmarks-dark.svg#gh-dark-mode-only">
    <img src="../resources/benchmarks/overview/h1-benchmarks-dark.svg" alt="Benchmarks">
  </a>
</h1>

This is where doba earns the "fast" bit. These adapters expose the routes
used by external HTTP/1.1 benchmarks; the upstream projects supply the load
and collect the results.

<a name="httparena"></a>
<h2>
  <a href="../resources/benchmarks/overview/h2-httparena.svg#gh-light-mode-only">
    <img src="../resources/benchmarks/overview/h2-httparena.svg" alt="HttpArena">
  </a>
  <a href="../resources/benchmarks/overview/h2-httparena-dark.svg#gh-dark-mode-only">
    <img src="../resources/benchmarks/overview/h2-httparena-dark.svg" alt="HttpArena">
  </a>
</h2>

The bundled HttpArena driver supports baseline, pipelined and real async delay
requests over plain HTTP. It registers both handler types by default and has
no TLS listener; the library offers TLS separately. A dynamic correctness group
checks the driver in root test builds; throughput measurement is separate.
[HttpArena adapter](http/v11/httparena/README.md).

<a name="web-frameworks"></a>
<h2>
  <a href="../resources/benchmarks/overview/h2-web-frameworks.svg#gh-light-mode-only">
    <img src="../resources/benchmarks/overview/h2-web-frameworks.svg" alt="Web Frameworks">
  </a>
  <a href="../resources/benchmarks/overview/h2-web-frameworks-dark.svg#gh-dark-mode-only">
    <img src="../resources/benchmarks/overview/h2-web-frameworks-dark.svg" alt="Web Frameworks">
  </a>
</h2>

A small adapter for the Web Frameworks benchmark: a root route, a route that
returns a user identifier, and a user POST route. Simple handlers, plenty
of traffic. [Web Frameworks adapter](http/v11/web-frameworks/README.md).

Current transport comparisons use the HttpArena adapter above. Removed
activity/flow-control simulations are not measurements of the current engine
or its native processing turns.
