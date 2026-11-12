<a name="unit-tests"></a>
<h1>
  <a href="../../resources/tests/unit/h1-unit-tests.svg#gh-light-mode-only">
    <img src="../../resources/tests/unit/h1-unit-tests.svg" alt="Unit tests">
  </a>
  <a href="../../resources/tests/unit/h1-unit-tests-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/unit/h1-unit-tests-dark.svg" alt="Unit tests">
  </a>
</h1>

An extra byte, an empty buffer, a handler that finishes at an awkward moment.
These tests check the small things that are easy to break while making
something else faster.

<a name="http-in-pieces"></a>
<h2>
  <a href="../../resources/tests/unit/h2-http-in-pieces.svg#gh-light-mode-only">
    <img src="../../resources/tests/unit/h2-http-in-pieces.svg" alt="HTTP in pieces">
  </a>
  <a href="../../resources/tests/unit/h2-http-in-pieces-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/unit/h2-http-in-pieces-dark.svg" alt="HTTP in pieces">
  </a>
</h2>

The [decoder](protocol/http/v11/decoder_tests.cpp), [body code](protocol/http/v11/body)
and [header rules](protocol/http/v11/headers/rules) get complete messages,
fragments and malformed input. A body at the limit and one byte over it are
different cases. So are "not finished yet" and "rejected".

[Routes and authentication](protocol/http/common) check who gets called.
[Serialization](protocol/http/v11/serialization_tests.cpp) checks the reply.
[Policy tests](protocol/http/v11/policy_behavior_tests.cpp) try zero, exact
boundaries, huge limits and combinations of settings.

<a name="who-owns-the-bytes"></a>
<h2>
  <a href="../../resources/tests/unit/h2-who-owns-the-bytes.svg#gh-light-mode-only">
    <img src="../../resources/tests/unit/h2-who-owns-the-bytes.svg" alt="Who owns the bytes?">
  </a>
  <a href="../../resources/tests/unit/h2-who-owns-the-bytes-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/unit/h2-who-owns-the-bytes-dark.svg" alt="Who owns the bytes?">
  </a>
</h2>

We overwrite caller buffers and read owned requests after the decoder is gone.
A pointer that happens to work today isn't much of a guarantee.

The [common tests](common) cover readers, writers, memory, disk spill and cleanup.
The [cache](common/cache_tests.cpp) also gets failed loads, eviction and
invalidation while another thread is loading. [Task tests](common/task_execution_tests.cpp)
check frame ownership, cancellation and deadlines without waiting for the clock.

[Transport helpers](transport) get partial sends, failed reads, empty output
and invalid settings. Linux and Windows each test the pieces they actually use.

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

[Engine cases](protocol/http/v11/engine_concurrency_tests.cpp) hold handlers at
known points, finish replies out of order and check that the wire order stays
right. They also check progress while a reader drains, per-turn work limits
and cancellation.

[HTTP budgets](protocol/http/v11/policy_budget_audit_tests.cpp) check that
queued work stays bounded and can resume. Real socket scheduling lives in the
[integration suite](../integration/README.md). The [mutation checks](../mutation/README.md)
make sure selected bugs really do trip an assertion.

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

The [runner tests](helpers/test_helper_tests.cpp) try bad assertions, exceptions,
filters and stuck cleanup. Concurrent failures must all show up. Temporary
directory settings must be restored, even when a test throws.

The [integration runner](../integration/helpers/test_helper_tests.cpp) also
checks that a failed assertion releases a blocked handler before joining its
threads. Nobody wants to debug a test runner with a test runner.
