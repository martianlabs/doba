# CI processing coverage

This inventory describes the configured workflow, not a new GitHub run.
[ci.yml](../.github/workflows/ci.yml) retains its existing job identifiers,
required-check names, compiler/sanitizer axes, triggers, permissions and release
prerequisites. New processing checks run inside those jobs.

| Area | Required coverage |
| --- | --- |
| Common and engine unit binary | Controlled task owner, processing result contract, concurrent engine cases and processing-turn cases; Linux also includes the ready/deadline registry tests. |
| Portable integration | Factory configuration, close precedence, RX/TX/reader rules and policy limits in the ordinary integration binary. |
| Linux native processing | doba_linux_processing_tests alongside transport and accept-fault groups; transport and processing repeat under TSan. |
| Windows native processing | Actual processing cases in the integration binary and doba_windows_processing_tests for forced early publication/cancellation/faults; repeated with IOCP/lifecycle probes under Windows sanitizers. |
| TLS | Existing wire/core/configuration/header/example groups plus doba_tls_processing_tests and doba_tls_processing_wire_tests. Dedicated TLS jobs explicitly build doba_tls_processing and execute both new groups. |
| Executable processing examples | doba_processing_examples_tests runs the standalone task owner, in-memory concurrent engine and native TCP example. Required when examples are enabled. |
| HttpArena driver | doba_httparena_tests checks metadata and actual baseline/pipelined/async behavior with dynamic values on isolated ports. It is always part of root test builds, not a throughput campaign. |
| Existing broader gates | Minimum CMake, package/source consumers, fuzzers and private-copy mutation campaigns remain configured; final qualification belongs to async-plan block 10. |

[ci_test_inventory.cmake](../cmake/ci_test_inventory.cmake) requires Release,
records selected build options, lists the registered unit/integration cases and
native probe inventories, then checks mandatory groups. It requires TLS groups
when TLS is requested and executable examples when enabled. This is a minimum
inventory guard, not a line-coverage score or proof that every possible behavior
is tested. The subsequent CTest run executes the complete registered suite.

Run locally without a Git lookup:

```text
cmake -DDOBA_CI_BUILD_DIR=<release-build> -DDOBA_CI_REQUIRE_TLS=ON
      -DDOBA_CI_SOURCE_DESCRIPTION=local-source-snapshot
      -P cmake/ci_test_inventory.cmake
```

Use one command line when running it. Build normally first; a dedicated TLS
job builds only its named TLS targets and checks them with explicit CTest
filters and --no-tests=error. Root jobs build all test targets and use the full
inventory guard. Debug jobs retain their existing execution scope.

Normal suite execution in Release is the intended policy. The security audit
does not require normal Debug execution. Inventory now also requires
`doba_security_model_tests`, `doba_stream_integrity_tests` and
`doba_test_diagnostics_tests`, plus their source fixtures. The first checks
144 bounded engine configurations; the second independently validates binary
pipelines under default admission limits; the third checks sanitizer-report
propagation from exit-zero child processes. Native sanitizer repetitions include
stream integrity, and concurrency repetitions include the state model.

Decoder fuzz jobs additionally replay twelve canonical engine seeds and run
`fuzz_engine` for 60 seconds on each OS. Job identifiers, matrix entries and
required-check names remain unchanged. These are configured gates; local
qualification and remote workflow execution must be distinguished.

Evidence is written under <build>/ci-evidence and uploaded by the workflow.
The current block-9 local checks also use private CTest inventories omitting,
one at a time, native processing, driver, TLS processing wire and executable
examples. Each omission must fail its intended diagnostic on Linux and Windows;
a generic process failure cannot count as a successful negative check.

## Measurement and reliability boundaries

The [driver check](integration/benchmarks/httparena.py) draws path values after
server startup and verifies their bodies, text/plain and minimum waits. It also
checks zero, different overlapping delays, mixed same-connection order, dynamic
GET/POST, chunked/fragmented bodies and sixteen-response pipelines. It reports
batch elapsed time without asserting a throughput target.

Minimum-delay checks use perf_counter_ns. Python <=3.12 on Windows can expose a
15.6 ms GetTickCount64 monotonic clock; using that clock to judge a 17 ms delay
can falsely report 16 ms. The high-resolution monotonic performance counter
avoids that problem without allowing an early response. Socket/process waits
remain bounded. Driver stderr is checked for sanitizer diagnostics even when
the harness terminates the benchmark-owned process; a reported race must not
be lost merely because that process did not exit normally. No tight concurrent
batch latency limit is imposed.

These checks do not activate an async mode in the library and do not configure
an injected executor. Both route kinds are registered in the driver and examples
by default. Published benchmark data and the frozen pre-async reference remain
unchanged; final throughput comparisons are a separate campaign.

## Release coverage and systematic qualification

[Qualification](qualification/README.md) adds separate Linux/Windows Release
source-coverage jobs, reviewed module floors and explicit mapped/unmapped
header inventories. Previously measured headers cannot disappear, and new
unmapped headers require review. Per-target profiles prevent cross-executable
counter contamination. The release job also depends on coverage; repository
rules must select the two new coverage checks separately after their first run.

The diagnostic qualification group is mandatory on both platforms; Windows
also requires isolated Winsock failure/cleanup tests. Existing sanitizer jobs
run seeded case-order campaigns (two rounds normally, twelve nightly).
Fuzzers retain 60-second normal budgets and use 600 seconds nightly, with a
separate persisted engine corpus. Dedicated TLS repetitions retain their scope.
See the [measured reference](qualification/BASELINE.md) for actual local results
and the distinction between emitted mappings, condition outcomes and safety.
