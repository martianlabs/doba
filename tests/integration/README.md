<a name="integration-tests"></a>
<h1>
  <a href="../../resources/tests/integration/h1-integration-tests.svg#gh-light-mode-only">
    <img src="../../resources/tests/integration/h1-integration-tests.svg" alt="Integration tests">
  </a>
  <a href="../../resources/tests/integration/h1-integration-tests-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/integration/h1-integration-tests-dark.svg" alt="Integration tests">
  </a>
</h1>

The pieces work on their own. Now put them behind a real socket and give the
client a chance to be difficult.

<a name="real-connections"></a>
<h2>
  <a href="../../resources/tests/integration/h2-real-connections.svg#gh-light-mode-only">
    <img src="../../resources/tests/integration/h2-real-connections.svg" alt="Real connections">
  </a>
  <a href="../../resources/tests/integration/h2-real-connections-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/integration/h2-real-connections-dark.svg" alt="Real connections">
  </a>
</h2>

The [HTTP tests](protocol/http/v11) cover routes, authentication, static files
and limits over TCP. Clients split messages, pipeline requests, half-close and
reset connections. A rejected request must not sneak its successor into a handler.

The [transport tests](transport) check Linux epoll/SO_REUSEPORT and Windows IOCP:
input buffers, partial output, reader ownership and failure cleanup.
[Policy cases](transport/tcp_policy_limits_tests.cpp) fill the connection limit,
reject extra clients and check that admitted clients still work. Freed slots
must be usable again without a restart.

[TLS has its own groups](tls/README.md). Both transports follow the
[TCP contract](../../docs/reference/transport/tcp.md#runtime-contract): the
transport delivers processing turns; the engine owns its coroutines and reply order.

<a name="drain-it-start-it-again"></a>
<h2>
  <a href="../../resources/tests/integration/h2-drain-it-start-it-again.svg#gh-light-mode-only">
    <img src="../../resources/tests/integration/h2-drain-it-start-it-again.svg" alt="Drain it. Start it again">
  </a>
  <a href="../../resources/tests/integration/h2-drain-it-start-it-again-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/integration/h2-drain-it-start-it-again-dark.svg" alt="Drain it. Start it again">
  </a>
</h2>

[Linux](transport/tcpip_linux_tests.cpp) gets slow readers, half-closes, failed
factories, partial startup and restarts. One [isolated test](transport/tcp_accept_error_linux.cpp)
uses up its own file descriptors, then checks failure and recovery. It leaves
the rest of the machine's limits alone.

[Windows drain tests](transport/tcpip_drain_windows_tests.cpp) check pending
output through stop and destruction. Separate probes force awkward
[IOCP completions](transport/tcp_iocp_completion_windows_probe.cpp),
[startup failures](transport/tcp_lifecycle_windows_probe.cpp) and
[timer events](transport/tcp_processing_windows_probe.cpp).
Those API replacements stay inside the test executables.

The [restart fixture](protocol/http/v11/server_resilience_windows_tests.cpp)
mixes slow clients, resets and disk spill, then checks replies, temporary files
and handles. Stop differs by platform: Linux cancels pending output after the
active call returns; Windows drains it. Neither promises a stop timeout.

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

The [runner checks](helpers/test_helper_tests.cpp) make failures, timeouts and
stuck cleanup name the offending test. [Response-parser checks](helpers/http_test_helper_tests.cpp)
feed malformed replies to the helper itself: a lenient checker can hide a bug.
Child-process sanitizer reports must survive cleanup too.

The [HttpArena check](benchmarks/httparena.cpp) and
[example checks](helpers/processing_examples.cpp) run the actual built programs.
They check correctness; throughput measurements are a separate job.
See [what CI runs](../ci-inventory.md) for the groups and repeat counts.
