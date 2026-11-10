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

The [security and integrity audit](security-audit.md) records verified test
weaknesses, strengthened oracles, the coverage matrix and remaining assurance
gaps. Normal suite execution in Release is intentional. Passing tests do not
certify the absence of races or vulnerabilities.

[Release qualification](qualification/README.md) explains measured production
coverage, per-module regression gates and reproducible sanitizer/fuzz campaigns.

All test runners and project tools must be implemented in C++20. They must not
require Python or another interpreter. CMake and workflow YAML configure builds
and CI steps. Native tool controls enforce the source policy and exercise child
diagnostics, argument quoting, deadlines, stdin and large captured output.

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

Real synchronous TCP connections, fragmented requests, inline pipelines,
disconnects, prefix/source and shutdown with socket output pending. Clients aren't obliged to be polite.
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

libFuzzer supplies questionable input. Decoder and chunked-body checks cover
boundaries, fragmentation, ownership and memory/disk storage. The engine model
checks mixed ordered responses, source lifetimes and terminal frontiers.
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

We plant eight bugs in private header copies. The original tests must pass;
each broken version must trip its intended assertion. Green needs a sanity
check too. [The planted bugs](mutation/README.md).

<a name="package-tests"></a>
<h2>
  <a href="../resources/tests/overview/h2-package-tests.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-package-tests.svg" alt="Package tests">
  </a>
  <a href="../resources/tests/overview/h2-package-tests-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-package-tests-dark.svg" alt="Package tests">
  </a>
</h2>

Separate projects consume installed and source CMake targets and construct
public HTTP/TCP/HTTPS types. Plain consumers do not import TLS dependencies.
The [TLS integration groups](integration/tls/README.md) exercise negotiation,
BIO progress, KeyUpdate, source limits and HTTPS pipelining.
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

Native Linux and Windows transport coverage follows the
[TCP runtime contract](../docs/reference/transport/tcp.md#runtime-contract).
Protocol-only async tests supply serialized processing turns and controlled
eligibility times. Native TCP and TLS groups validate real-socket coroutine
handlers, mixed ordered output, cancellation and independent connection progress.
Common::task standalone scheduling remains a separate API. Inactivity and
pressure/resume remain outside the transport contract.

CI preserves historical Release TLS required-check names for branch protection,
and keeps those TCP jobs configured with TLS=OFF. Dedicated TLS jobs build and
run the current TLS tests and examples with TLS=ON. TCP jobs validate the native
cases listed in the suite documentation. Engine concurrency repeats 200 times;
native Linux transport/processing or Windows IOCP/processing groups repeat
20 times in their jobs. Dedicated TLS jobs build the processing probe and run
both processing groups alongside the existing TLS coverage.
Local inventory can be collected without Git by providing
-DDOBA_CI_SOURCE_DESCRIPTION=working-tree to cmake/ci_test_inventory.cmake and
preserving source hashes. These statements describe configured coverage, not a
new remote CI run.

Protocol processing-contract tests qualify the result value, exact entry-point
return types and unchanged factory construction without HTTP dependencies.
TCP holder tests verify complete RX delivery and buffer reuse. Native tests
exercise immediate/deadline/output-drain interests, coalescing, cancellation,
publication interleavings and restart. TLS processing checks preserve interests
through control records and advance handlers during encrypted output drain.
CI inventory requires the current native and TLS groups, the HttpArena dynamic
correctness test and actual executable processing examples when built.

The [async retirement inventory](../docs/reference/protocol/http/v11/async-retirement.md)
maps removed execution/pressure fixtures to current processing and admission
coverage. Tests do not preserve unsupported contracts solely as fake hooks.

See [CI processing coverage](ci-inventory.md) for mandatory groups, configured
sanitizer runs and the separation between correctness and throughput checks.
