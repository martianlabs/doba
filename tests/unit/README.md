<a name="unit-tests"></a>
<h1>
  <a href="../../resources/tests/unit/h1-unit-tests.svg#gh-light-mode-only">
    <img src="../../resources/tests/unit/h1-unit-tests.svg" alt="Unit tests">
  </a>
  <a href="../../resources/tests/unit/h1-unit-tests-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/unit/h1-unit-tests-dark.svg" alt="Unit tests">
  </a>
</h1>

An extra byte, an empty buffer, a handler that finishes at the worst possible
moment. The unit suite checks the small contracts that are very easy to break
while making something else faster.

<a name="http-in-pieces"></a>
<h2>
  <a href="../../resources/tests/unit/h2-http-in-pieces.svg#gh-light-mode-only">
    <img src="../../resources/tests/unit/h2-http-in-pieces.svg" alt="HTTP in pieces">
  </a>
  <a href="../../resources/tests/unit/h2-http-in-pieces-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/unit/h2-http-in-pieces-dark.svg" alt="HTTP in pieces">
  </a>
</h2>

The [decoder tests](protocol/http/v11/decoder_tests.cpp),
[body tests](protocol/http/v11/body), and
[header rules](protocol/http/v11/headers/rules) cover valid messages,
malformed framing, fragmentation, pipelines, and exact policy boundaries.
A body at the limit and a body one byte over it are different cases. So are
an incomplete message and a rejected one.

[Routing and authentication](protocol/http/common) cover route selection,
parameters, controllers, asynchronous handlers, and credential parsing.
[Serialization checks](protocol/http/v11/serialization_tests.cpp) cover the
response bytes that come out the other end.

<a name="who-owns-the-bytes"></a>
<h2>
  <a href="../../resources/tests/unit/h2-who-owns-the-bytes.svg#gh-light-mode-only">
    <img src="../../resources/tests/unit/h2-who-owns-the-bytes.svg" alt="Who owns the bytes?">
  </a>
  <a href="../../resources/tests/unit/h2-who-owns-the-bytes-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/unit/h2-who-owns-the-bytes-dark.svg" alt="Who owns the bytes?">
  </a>
</h2>

The [cache tests](common/cache_tests.cpp) exercise bounded cost, expiration,
shared loading and invalidation races independently of HTTP. The
[common tests](common) check readers, writers, filesystem operations,
containers, and coroutine helpers. Storage cases exercise memory buffers,
disk spill, moves, cleanup, and failed operations.

Decoder cases also reuse or overwrite caller buffers and inspect owned
requests after the decoder has gone away. A view that happens to point at
something readable today isn't much of an ownership guarantee.

[Transport tests](transport/server) check connection behavior and invalid
policies. [Linux context tests](transport/server/tcp_context_linux_tests.cpp)
cover bounded RX/TX, partial sends, EAGAIN, sources, empty sends and closure.
The [Windows TX state](transport/server/send_state_tests.cpp) and
[RX adapter](transport/server/tcp_connection_tests.cpp) are tested independently.
These helper tests do not assert that Linux uses those Windows components.

<a name="bad-timing-on-purpose"></a>
<h2>
  <a href="../../resources/tests/unit/h2-bad-timing-on-purpose.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/unit/h2-bad-timing-on-purpose-narrow.svg">
      <img src="../../resources/tests/unit/h2-bad-timing-on-purpose.svg" alt="Bad timing, on purpose">
    </picture>
  </a>
  <a href="../../resources/tests/unit/h2-bad-timing-on-purpose-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/unit/h2-bad-timing-on-purpose-narrow-dark.svg">
      <img src="../../resources/tests/unit/h2-bad-timing-on-purpose-dark.svg" alt="Bad timing, on purpose">
    </picture>
  </a>
</h2>

The [engine concurrency cases](protocol/http/v11/engine_concurrency_tests.cpp)
use explicit gates and independently supplied execution services to control
handler completion, input and protocol stop. They verify ordering, owned frames,
callback counts and lifetimes. This is engine coverage, not coroutine support
in the TCP transports. Common coroutine helpers remain independently tested.

[Operating helpers](transport/server/operating_policies_tests.cpp) test shared
Windows admission reservations and pressure calculations used by the engine.
[HTTP budgets](protocol/http/v11/policy_budget_audit_tests.cpp) and
[flow wakeups](protocol/http/v11/flow_wakeup_tests.cpp) exercise independent
protocol credit and readiness, including subscription/completion races.
The TCP backends do not install those hooks or perform inactivity sampling.

[Policy construction](transport/server/tcp_transport_tests.cpp) covers defaults,
platform ranges and zero meanings. [Regression detectors](protocol/http/v11/regression_detection_tests.cpp)
check framing/body budgets, ordered responses and terminal rejection. The
[configured mutation campaign](../mutation/README.md) challenges twelve selected
regressions in private copies; it does not mutate the working source tree.

<a name="the-runner-gets-checked-too"></a>
<h2>
  <a href="../../resources/tests/unit/h2-the-runner-gets-checked-too.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/unit/h2-the-runner-gets-checked-too-narrow.svg">
      <img src="../../resources/tests/unit/h2-the-runner-gets-checked-too.svg" alt="The runner gets checked too">
    </picture>
  </a>
  <a href="../../resources/tests/unit/h2-the-runner-gets-checked-too-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/unit/h2-the-runner-gets-checked-too-narrow-dark.svg">
      <img src="../../resources/tests/unit/h2-the-runner-gets-checked-too-dark.svg" alt="The runner gets checked too">
    </picture>
  </a>
</h2>

The [runner metatests](helpers/test_helper_tests.cpp) check listing,
selection, assertion reports, exceptions, exit status, and timeouts.
They also check that a failed assertion releases suspended work before
joining threads, and that blocked cleanup leaves the test's identity in
the diagnostics. A broken test shouldn't take the rest of the suite hostage.

Failure counters and context diagnostics are synchronized across test workers.
The metatests require all 128 failures from eight concurrent workers to be
reported. Temporary directory overrides restore the prior environment during
unwinding; decoder spill checks use private directories and propagate directory
inspection errors instead of treating them as empty results.

