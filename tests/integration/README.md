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

The [HTTP cases](protocol/http/v11) exercise synchronous routing, authentication,
controllers, limits, static files and ordered inline responses over TCP. Clients
fragment requests, pipeline messages, half-close and reset real connections.
Rejected input must not dispatch a later handler on that connection.

The [native transport cases](transport) check Linux epoll/SO_REUSEPORT
and Windows IOCP under the [TCP contract](../../docs/reference/transport/tcp.md#runtime-contract).
They cover fresh RX blocks, engine-owned fragments, bounded/partial TX, one
terminal reader, failure isolation, startup/stop and restart.
The separate [TLS groups](tls/README.md) cover negotiation, encrypted traffic,
configuration, processing turns, ordered mixed handlers, limits and both TLS
executable examples on the same backends.
Native transports deliver generic processing turns; coroutine frames and
response order belong to the engine. Pressure/resume remains outside scope.

[Portable empty-send cases](transport/tcp_empty_source_tests.cpp) run in
both binaries with one/four workers and an exact TX budget: reader or empty
reader, continuation, close, and rejection of a second reader or later bytes.
[Portable policy cases](transport/tcp_policy_limits_tests.cpp) check full
endpoint strings, NUL rejection, snapshots, RX bounds, default workers, backlog,
zero/finite admission, exact TX, overflow and recovery.

[Filesystem cases](common/filesystem_tests.cpp) exercise real readers, ownership
moves and changes made before a first read. They are independent of socket I/O.

<a name="drain-it-start-it-again"></a>
<h2>
  <a href="../../resources/tests/integration/h2-drain-it-start-it-again.svg#gh-light-mode-only">
    <img src="../../resources/tests/integration/h2-drain-it-start-it-again.svg" alt="Drain it. Start it again">
  </a>
  <a href="../../resources/tests/integration/h2-drain-it-start-it-again-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/integration/h2-drain-it-start-it-again-dark.svg" alt="Drain it. Start it again">
  </a>
</h2>

[Linux cases](transport/tcpip_linux_tests.cpp) exercise independent
workers, partitioned admission, half-close/source drain, slow peers, factory and
notification failures, partial-start rollback, descriptors and restart. Stop
cancels pending output after the current engine/reader call returns. The isolated
[accept-error executable](transport/tcp_accept_error_linux.cpp) exhausts
only its own descriptor limit, checks admission failure and explicit recovery,
and restores resources. No global runner limits are changed.

[Windows drain cases](transport/tcpip_drain_windows_tests.cpp) verify
accepted output through stop/destruction, bounded readers, source failures,
accept bursts, paired notifications and restart. [Policies](transport/operating_policies_windows_tests.cpp),
[policy snapshots](transport/policy_audit_windows_tests.cpp) and
[admission](transport/admission_windows_tests.cpp) cover real listeners.
[Prefix/source](transport/tcp_prefix_source_windows_tests.cpp) checks
reader release before new input; [serialization](transport/tcp_serialization_windows_tests.cpp)
checks many synchronous output callbacks and progress on independent connections.
[Factory state](transport/tcp_factory_tests.cpp) verifies the current
engine contract without execution hooks on Linux and Windows: one factory per
accepted connection, preserved application state and paired disconnects with
one/two workers.

Isolated Windows executables intercept APIs only inside their own tests:
[IOCP publication](transport/tcp_iocp_completion_windows_probe.cpp) forces
packet dequeue before the socket call returns; [lifecycle](transport/tcp_lifecycle_windows_probe.cpp)
checks partial startup, socket ownership, failed submissions and restart.
[Processing publication](transport/tcp_processing_windows_probe.cpp)
forces early dequeue, stale timer wake cancellation, batches and native faults.
Production and benchmark sources have no hooks. CI lists both executables and
repeats their cases; historical required-check labels containing TLS still
configure TLS=OFF.

The [Windows resilience fixture](protocol/http/v11/server_resilience_windows_tests.cpp)
uses synchronous TCP, inline pipelines, slow clients, resets and body spill.
It checks response order, cleanup, temporary files and handles across restart.
Resource observations after warmup are evidence, not a universal RSS ceiling.
Engine/common-task asynchronous lifecycle tests remain separate from sockets.

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

The [runner metatests](helpers/test_helper_tests.cpp) check listing, exact and
qualified filters, repeated exclusions, assertions, exceptions, timeouts and
cleanup in child processes. An assertion during an active fixture must release
its synchronous handler's gate before joining workers. The watchdog identifies
the blocked case even
when cleanup cannot progress. Concurrent reports retain every failure.

The [HTTP helper regressions](helpers/http_test_helper_tests.cpp) verify raw-wire
rejection of mixed-case duplicate Content-Length, Transfer-Encoding and Date.
Native TCP helpers use elapsed-time deadlines and retry interruptions; Linux
uses poll for high descriptors. The TLS groups use independent, verified C++
OpenSSL clients; they do not restore the retired platform-specific TLS clients.

Platform-specific files are selected by CMake; unsupported or absent scenarios
are not counted as passing. Shared portable cases must appear in both native
integration inventories. See the [policy coverage matrix](../../docs/reference/transport/tcp_policies.md#validation-coverage).


The [HttpArena correctness group](benchmarks/httparena.cpp) exercises the actual
standalone driver with all three selected profiles.
[Processing example checks](helpers/processing_examples.cpp) execute the built
common, in-memory engine and TCP demos when examples are enabled. Neither group
substitutes for the final throughput comparison. See [CI coverage](../ci-inventory.md).
