<a name="integration-tests"></a>
<h1>
  <a href="../../resources/tests/integration/h1-integration-tests.svg#gh-light-mode-only">
    <img src="../../resources/tests/integration/h1-integration-tests.svg" alt="Integration tests">
  </a>
  <a href="../../resources/tests/integration/h1-integration-tests-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/integration/h1-integration-tests-dark.svg" alt="Integration tests">
  </a>
</h1>

A parser can pass every carefully arranged input and still end up inside a
server that mishandles real connections. This suite puts the pieces together
and gives the clients plenty of opportunities to be inconvenient.

<a name="real-connections"></a>
<h2>
  <a href="../../resources/tests/integration/h2-real-connections.svg#gh-light-mode-only">
    <img src="../../resources/tests/integration/h2-real-connections.svg" alt="Real connections">
  </a>
  <a href="../../resources/tests/integration/h2-real-connections-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/integration/h2-real-connections-dark.svg" alt="Real connections">
  </a>
</h2>

The [HTTP server cases](protocol/http/v11) exercise routing, authentication,
controllers, request limits, static files, and response ordering over TCP.
Requests arrive fragmented or pipelined; clients disconnect while work is
pending; handlers suspend and resume before the server shuts down.

Security cases check rejection boundaries, including a hostile request
followed by another request on the same connection. Rejecting one message
mustn't leave a back door into the next handler.

The [transport cases](transport/server) check TCP draining and TLS behavior:
negotiation, fragmented encrypted input, binary HTTP bodies, bounded
buffers, sequential requests, and asynchronous work. Native transport
scenarios exercise Linux startup coordination and Windows IOCP workers.

[Filesystem cases](common/filesystem_tests.cpp) check real file readers,
including independent cursors after ownership moves and changes made before
the first read.

<a name="drain-it-start-it-again"></a>
<h2>
  <a href="../../resources/tests/integration/h2-drain-it-start-it-again.svg#gh-light-mode-only">
    <img src="../../resources/tests/integration/h2-drain-it-start-it-again.svg" alt="Drain it. Start it again">
  </a>
  <a href="../../resources/tests/integration/h2-drain-it-start-it-again-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/integration/h2-drain-it-start-it-again-dark.svg" alt="Drain it. Start it again">
  </a>
</h2>

The [operating policies](transport/server/operating_policies_tests.cpp),
[flow closure](transport/server/flow_close_tests.cpp), and
[policy audit](transport/server/policy_audit_tests.cpp) exercise real TCP/TLS
listeners: admission and recovery, zero limits and configuration snapshots,
receive/send progress, paused pipelines, slow readers, held async work,
wakeups during receive, and TLS close notifications during resumption.
TLS 1.2 and 1.3 handshakes also run with a 256-byte output BIO, forcing
retries after output drains while HTTP input remains unpaused. Each
connection must finish negotiation and serve an HTTP response.
The [flow pressure tests](transport/server/flow_pressure_tests.cpp) verify immediate
publication after enqueue, release after drain and safe queries after retirement.
The [operating policy cases](transport/server/operating_policies_tests.cpp) and
[policy audit](transport/server/policy_audit_tests.cpp) cover boundaries,
inactivity, retained work and concurrent flow control. The
[mutation campaign](../mutation/README.md) checks regression detection separately.

The [resilience cases](protocol/http/v11/server_resilience_tests.cpp) mix
several TCP or TLS clients, tiny reads, abrupt resets, suspended handlers,
and bodies that spill to disk. The server drains, restarts, and repeats
the cycle.

The checks account for payload order, handler completion and destruction,
and leftover temporary files. Handle or descriptor counts must not grow
after draining. Memory use is recorded after warmup as well; that reading
is evidence to inspect, rather than a fixed memory ceiling assertion.

<a name="when-a-test-goes-wrong"></a>
<h2>
  <a href="../../resources/tests/integration/h2-when-a-test-goes-wrong.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/integration/h2-when-a-test-goes-wrong-narrow.svg">
      <img src="../../resources/tests/integration/h2-when-a-test-goes-wrong.svg" alt="When a test goes wrong">
    </picture>
  </a>
  <a href="../../resources/tests/integration/h2-when-a-test-goes-wrong-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/integration/h2-when-a-test-goes-wrong-narrow-dark.svg">
      <img src="../../resources/tests/integration/h2-when-a-test-goes-wrong-dark.svg" alt="When a test goes wrong">
    </picture>
  </a>
</h2>

The [integration runner metatests](helpers/test_helper_tests.cpp) check
filters, failure reports, exceptions, deadlines, and cleanup using child
processes. They include assertion failures with an active transport and
suspended work, so the error path has to clean up too.

A timeout must identify the blocked test, including when its cleanup gets
stuck. "Something hung somewhere" is not a useful test result.

Both runners apply every repeated `--exclude`; qualified exclusions use
`tests/integration/path.cpp::test name` and accept either path separator.
Concurrent assertion reports are checked by child-process metatests.

The HTTP response oracle has independent raw-wire regressions for mixed-case
duplicate Content-Length, Transfer-Encoding and Date fields. The TLS test client
uses elapsed-time deadlines rather than iteration budgets and respects inbound
BIO capacity. A response with more than 400 small TLS records exercises this
boundary. TCP helper reads retry interrupted operations against the original
deadline; Linux readiness uses `poll` to support high descriptors safely.

The Windows [input resume regression](transport/server/flow_resume_windows_tests.cpp)
keeps one connection through 32 pressured pipelines with 64 responses each.
It checks ordered credit, complete responses and handler counts while two IOCP
workers race output completions against input resumption. Input ownership must
be released before arming output, so an immediate send completion can resume
buffered requests instead of losing its `try_lock` attempt.
