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

The [common tests](common) check readers, writers, filesystem operations,
containers, and coroutine helpers. Storage cases exercise memory buffers,
disk spill, moves, cleanup, and failed operations.

Decoder cases also reuse or overwrite caller buffers and inspect owned
requests after the decoder has gone away. A view that happens to point at
something readable today isn't much of an ownership guarantee.

[Transport tests](transport/server) check connection behavior and invalid
policies. TLS session cases cover credentials, fragmented negotiation,
application data, buffer limits, and close notifications.

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

The [concurrency tests](protocol/http/v11/engine_concurrency_tests.cpp)
suspend handlers and control completion, receive, and shutdown overlaps.
They check response ordering, callback counts, and object
lifetimes instead of hoping the scheduler produces an interesting race.

The [regression detectors](protocol/http/v11/regression_detection_tests.cpp)
check body limits, queued responses, and terminal rejection. Those same
assertions are challenged by the [mutation checks](../mutation/README.md).

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
