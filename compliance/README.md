<a name="compliance"></a>
<h1>
  <a href="../resources/compliance/overview/h1-compliance.svg#gh-light-mode-only">
    <img src="../resources/compliance/overview/h1-compliance.svg" alt="Compliance">
  </a>
  <a href="../resources/compliance/overview/h1-compliance-dark.svg#gh-dark-mode-only">
    <img src="../resources/compliance/overview/h1-compliance-dark.svg" alt="Compliance">
  </a>
</h1>

A second opinion on doba's HTTP/1.1 manners. These adapters let external
suites send the requests and judge the responses. We don't get to mark our
own homework here.

<a name="h1spec"></a>
<h2>
  <a href="../resources/compliance/overview/h2-h1spec.svg#gh-light-mode-only">
    <img src="../resources/compliance/overview/h2-h1spec.svg" alt="h1spec">
  </a>
  <a href="../resources/compliance/overview/h2-h1spec-dark.svg#gh-dark-mode-only">
    <img src="../resources/compliance/overview/h2-h1spec-dark.svg" alt="h1spec">
  </a>
</h2>

Checks HTTP/1.1 behavior against a running doba server. The adapter supplies
the method and body-echo routes expected by the suite.
[h1spec adapter](http/v11/h1spec/README.md).

<a name="http11probe"></a>
<h2>
  <a href="../resources/compliance/overview/h2-http11probe.svg#gh-light-mode-only">
    <img src="../resources/compliance/overview/h2-http11probe.svg" alt="Http11Probe">
  </a>
  <a href="../resources/compliance/overview/h2-http11probe-dark.svg#gh-dark-mode-only">
    <img src="../resources/compliance/overview/h2-http11probe-dark.svg" alt="Http11Probe">
  </a>
</h2>

Exercises the server with HTTP/1.1 probes, using routes that expose request
bodies, received headers, and parsed cookies. Awkward questions deserve
inspectable answers. [Http11Probe adapter](http/v11/http11probe/README.md).
