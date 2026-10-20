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
