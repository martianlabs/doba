<a name="ci-processing-coverage"></a>
<h1>
  <a href="../resources/tests/ci-inventory/h1-ci-processing-coverage.svg#gh-light-mode-only">
    <img src="../resources/tests/ci-inventory/h1-ci-processing-coverage.svg" alt="What CI runs">
  </a>
  <a href="../resources/tests/ci-inventory/h1-ci-processing-coverage-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/ci-inventory/h1-ci-processing-coverage-dark.svg" alt="What CI runs">
  </a>
</h1>

A green job is less useful if it quietly stopped building half the tests.
The [inventory check](../cmake/ci_test_inventory.cmake) lists what was built
and refuses to continue when a required group is missing.

| Area | What runs |
| --- | --- |
| Unit tests | Tasks, HTTP engine, policies and the fixed engine model. |
| Integration tests | Real HTTP/TCP traffic, policy limits and binary reply checks. |
| Linux probes | Transport, processing turns and accept failures. |
| Windows probes | IOCP completion, startup, timer/cancellation and Winsock failures. |
| TLS builds | Core, sockets, configuration, processing and both example programs. |
| Examples and driver | Built processing examples and the actual HttpArena driver. |
| Other checks | Package consumers, minimum CMake, fuzzing, mutations and source coverage. |

Root jobs build the full suite, check the inventory, then run CTest. Examples
are required when enabled; TLS groups are required when TLS is requested.
Dedicated TLS jobs build and run their named targets separately.

To check a local Release build without looking up a Git revision:

```sh
cmake -DDOBA_CI_BUILD_DIR=<release-build> -DDOBA_CI_REQUIRE_TLS=ON -DDOBA_CI_SOURCE_DESCRIPTION=local-source-snapshot -P cmake/ci_test_inventory.cmake
```

Set `DOBA_CI_REQUIRE_TLS=OFF` for a plain TCP build. Lists and build options
go to `<build>/ci-evidence`; the workflow uploads them with the test logs.
We also try inventories with required groups removed and check that they fail
for the right reason.

<a name="measurement-and-reliability-boundaries"></a>
<h2>
  <a href="../resources/tests/ci-inventory/h2-measurement-and-reliability-boundaries.svg#gh-light-mode-only">
    <img src="../resources/tests/ci-inventory/h2-measurement-and-reliability-boundaries.svg" alt="Correct answers before fast answers">
  </a>
  <a href="../resources/tests/ci-inventory/h2-measurement-and-reliability-boundaries-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/ci-inventory/h2-measurement-and-reliability-boundaries-dark.svg" alt="Correct answers before fast answers">
  </a>
</h2>

The [HttpArena check](integration/benchmarks/httparena.cpp) tries dynamic GET/POST
values, chunked and fragmented bodies, sixteen-reply pipelines and overlapping
delays. It checks response order and minimum waits with a monotonic clock.
It doesn't demand a throughput score or a tight upper latency bound.

All child-process stderr is checked for sanitizer reports, including when the
runner stops the server itself. Otherwise a good reply could hide a bad shutdown.

<a name="release-coverage-and-systematic-qualification"></a>
<h2>
  <a href="../resources/tests/ci-inventory/h2-release-coverage-and-systematic-qualification.svg#gh-light-mode-only">
    <img src="../resources/tests/ci-inventory/h2-release-coverage-and-systematic-qualification.svg" alt="More than one green run">
  </a>
  <a href="../resources/tests/ci-inventory/h2-release-coverage-and-systematic-qualification-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/ci-inventory/h2-release-coverage-and-systematic-qualification-dark.svg" alt="More than one green run">
  </a>
</h2>

Linux TSan repeats engine/model groups 200 times and native socket/processing
groups 20 times. Windows ASan jobs repeat the matching engine and native groups.
Dedicated TLS groups get five runs.

Sanitizer jobs also run [seeded campaigns](qualification/README.md#systematic-campaigns).
[Fuzzing](fuzz/README.md) and [coverage checks](qualification/README.md#source-coverage)
have their own budgets. Coverage is a release prerequisite; required branch
checks still have to be selected in repository rules.

[ci.yml](../.github/workflows/ci.yml) is the source for the exact job matrix.
This page describes that setup. Recorded local results live in the
[coverage snapshot](qualification/BASELINE.md) and [policy checks](qualification/POLICIES.md).
