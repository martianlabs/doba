<a name="tests"></a>
<h1>
  <a href="../resources/tests/overview/h1-tests.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h1-tests.svg" alt="Tests">
  </a>
  <a href="../resources/tests/overview/h1-tests-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h1-tests-dark.svg" alt="Tests">
  </a>
</h1>

Fast code still has to behave. Here's where we check that doba does, from a
single borrowed byte to a server full of unhelpful clients.

<a name="unit-tests"></a>
<h2>
  <a href="../resources/tests/overview/h2-unit-tests.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-unit-tests.svg" alt="Unit tests">
  </a>
  <a href="../resources/tests/overview/h2-unit-tests-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-unit-tests-dark.svg" alt="Unit tests">
  </a>
</h2>

Parsers, routing, storage, ownership, and exact limits. That extra byte
somebody swore would never arrive gets its own test.
[Inside the unit suite](unit/README.md).

<a name="integration-tests"></a>
<h2>
  <a href="../resources/tests/overview/h2-integration-tests.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-integration-tests.svg" alt="Integration tests">
  </a>
  <a href="../resources/tests/overview/h2-integration-tests-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-integration-tests-dark.svg" alt="Integration tests">
  </a>
</h2>

Real TCP and TLS connections, fragmented requests, pipelines, disconnects,
and shutdown with work still pending. Clients aren't obliged to be polite.
[Inside the integration suite](integration/README.md).

<a name="resilience"></a>
<h2>
  <a href="../resources/tests/overview/h2-resilience.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-resilience.svg" alt="Resilience">
  </a>
  <a href="../resources/tests/overview/h2-resilience-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-resilience-dark.svg" alt="Resilience">
  </a>
</h2>

Slow readers, resets, disk spill, drains, and restarts. Then we check payload
order, handler cleanup, temporary files, and handles/descriptors. Surviving
the party includes cleaning up afterwards.
[Resilience checks](integration/README.md#drain-it-start-it-again).

<a name="fuzzing"></a>
<h2>
  <a href="../resources/tests/overview/h2-fuzzing.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-fuzzing.svg" alt="Fuzzing">
  </a>
  <a href="../resources/tests/overview/h2-fuzzing-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-fuzzing-dark.svg" alt="Fuzzing">
  </a>
</h2>

libFuzzer supplies the questionable HTTP. Decoder and chunked-body checks
cover boundaries, fragmentation, ownership, and memory/disk storage.
[What the fuzzers check](fuzz/README.md).

<a name="mutation-checks"></a>
<h2>
  <a href="../resources/tests/overview/h2-mutation-checks.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-mutation-checks.svg" alt="Mutation checks">
  </a>
  <a href="../resources/tests/overview/h2-mutation-checks-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-mutation-checks-dark.svg" alt="Mutation checks">
  </a>
</h2>

We plant three bugs in private header copies. The original tests must pass;
each broken version must trip its intended assertion. Green needs a sanity
check too. [The three planted bugs](mutation/README.md).

<a name="package-tests"></a>
<h2>
  <a href="../resources/tests/overview/h2-package-tests.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-package-tests.svg" alt="Package tests">
  </a>
  <a href="../resources/tests/overview/h2-package-tests-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-package-tests-dark.svg" alt="Package tests">
  </a>
</h2>

Separate projects consume doba's CMake targets and construct public HTTP/TLS
types. The library has to work outside its own backyard.
[Package checks](package/README.md).

<a name="runner-metatests"></a>
<h2>
  <a href="../resources/tests/overview/h2-runner-metatests.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-runner-metatests.svg" alt="Runner metatests">
  </a>
  <a href="../resources/tests/overview/h2-runner-metatests-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-runner-metatests-dark.svg" alt="Runner metatests">
  </a>
</h2>

Filters, failure reports, exceptions, deadlines, and cleanup get tested too.
A stuck test must leave something more useful than a spinning CI job.
[Unit runner](unit/README.md#the-runner-gets-checked-too) and
[integration runner](integration/README.md#when-a-test-goes-wrong).
