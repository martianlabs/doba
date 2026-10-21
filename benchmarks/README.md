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

Baseline requests, delayed responses, JSON generation and compression,
static files, and body echo, with HTTP and TLS routes. A few more things to
do than saying hello very quickly.
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
