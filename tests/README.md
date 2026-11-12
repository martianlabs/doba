<a name="tests"></a>
<h1>
  <a href="../resources/tests/overview/h1-tests.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h1-tests.svg" alt="Tests">
  </a>
  <a href="../resources/tests/overview/h1-tests-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h1-tests-dark.svg" alt="Tests">
  </a>
</h1>

Fast is nice. Sending the right bytes and cleaning up afterwards is nicer.
Here's where we check both.

From the repo root, with CMake, Ninja and a C++20 compiler ready:

```sh
cmake -S . -B out/build/tests -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build out/build/tests
ctest --test-dir out/build/tests --output-on-failure --no-tests=error
```

Add `-DDOBA_ENABLE_TLS=ON` at configure time for TLS; you'll need OpenSSL 3.0+.
On Windows, use a developer shell. Tests run in Release, and their checks stay
active there. Runners and test tools are C++20 too.

<a name="unit-tests"></a>
<h2>
  <a href="../resources/tests/overview/h2-unit-tests.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-unit-tests.svg" alt="Unit tests">
  </a>
  <a href="../resources/tests/overview/h2-unit-tests-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-unit-tests-dark.svg" alt="Unit tests">
  </a>
</h2>

Parsers, routes, storage and exact limits. That extra byte somebody swore would
never arrive gets its own test. [Take a look](unit/README.md).

<a name="integration-tests"></a>
<h2>
  <a href="../resources/tests/overview/h2-integration-tests.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-integration-tests.svg" alt="Integration tests">
  </a>
  <a href="../resources/tests/overview/h2-integration-tests-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-integration-tests-dark.svg" alt="Integration tests">
  </a>
</h2>

Real sockets, fragmented requests, pipelines and clients that vanish halfway
through a reply. [Put the pieces together](integration/README.md).

<a name="resilience"></a>
<h2>
  <a href="../resources/tests/overview/h2-resilience.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-resilience.svg" alt="Resilience">
  </a>
  <a href="../resources/tests/overview/h2-resilience-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-resilience-dark.svg" alt="Resilience">
  </a>
</h2>

Slow readers, resets, disk spill and restarts. We check the reply bytes and what
gets left behind. [Drain it, then start it again](integration/README.md#drain-it-start-it-again).

<a name="fuzzing"></a>
<h2>
  <a href="../resources/tests/overview/h2-fuzzing.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-fuzzing.svg" alt="Fuzzing">
  </a>
  <a href="../resources/tests/overview/h2-fuzzing-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-fuzzing-dark.svg" alt="Fuzzing">
  </a>
</h2>

libFuzzer brings questionable input. We check boundaries, ownership and whether
splitting a message changes its meaning. [Meet the fuzzers](fuzz/README.md).

<a name="mutation-checks"></a>
<h2>
  <a href="../resources/tests/overview/h2-mutation-checks.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-mutation-checks.svg" alt="Mutation checks">
  </a>
  <a href="../resources/tests/overview/h2-mutation-checks-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-mutation-checks-dark.svg" alt="Mutation checks">
  </a>
</h2>

We plant eight bugs in private copies and make the tests catch them.
[Green needs a sanity check too](mutation/README.md).

<a name="package-tests"></a>
<h2>
  <a href="../resources/tests/overview/h2-package-tests.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-package-tests.svg" alt="Package tests">
  </a>
  <a href="../resources/tests/overview/h2-package-tests-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-package-tests-dark.svg" alt="Package tests">
  </a>
</h2>

A separate project tries to use doba through its CMake targets.
[No special treatment for being in the same repo](package/README.md).

<a name="runner-metatests"></a>
<h2>
  <a href="../resources/tests/overview/h2-runner-metatests.svg#gh-light-mode-only">
    <img src="../resources/tests/overview/h2-runner-metatests.svg" alt="Runner metatests">
  </a>
  <a href="../resources/tests/overview/h2-runner-metatests-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/overview/h2-runner-metatests-dark.svg" alt="Runner metatests">
  </a>
</h2>

A broken test shouldn't take the suite hostage. We test filters, failure
reports, timeouts and cleanup in the [unit](unit/README.md#the-runner-gets-checked-too)
and [integration](integration/README.md#when-a-test-goes-wrong) runners.

Looking for a particular answer? See [what CI runs](ci-inventory.md),
[release checks](qualification/README.md), [policy tests](qualification/POLICIES.md)
or [what the test review found](security-audit.md). Passing runs are useful
evidence; they can't promise that every bug is gone.
