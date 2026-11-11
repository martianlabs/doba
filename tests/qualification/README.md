# Release qualification

Coverage and campaigns measure different properties. A covered line might
still contain a bug; a repeated test can still miss a rare schedule. These
tools preserve both the measurements and their limitations.

The [measured baseline](BASELINE.md) records the local reference and the gaps
that produced additional fault tests. Remote CI results remain separate.

The [policy inventory](POLICIES.md) maps every public TCP, TLS, HTTP and cache
policy to behavioral checks, including overload and recovery.

## Source coverage

`doba_coverage` configures an isolated Clang **Release** build with TLS and
examples, builds it and runs the complete registered CTest suite. Normal
Release test policy is unchanged. Instrumentation is private to this build;
it does not add flags to the installed targets or production headers.
Never use this build for throughput measurements.

The compiler uses LLVM source coverage and atomic counter updates. Matching
`clang++`, `llvm-profdata` and `llvm-cov` tools are required. The CI Linux job
uses LLVM 18; Windows pins LLVM 22.1.8 and verifies its installer SHA-256.
Windows also requires a developer environment, Ninja and OpenSSL on PATH.

All project test and qualification logic is C++20. Python is not required.
CMake and workflow YAML declare builds and CI steps; they do not replace the
native coverage or campaign implementation. Bootstrap the uninstrumented tools:

```text
cmake -S tests/qualification -B out/build/qualification -G Ninja
  -DCMAKE_BUILD_TYPE=Release
cmake --build out/build/qualification --parallel 2
```

Use one command line for the configure command. On Windows append `.exe` to
tool paths. Root test builds also build both tools in `tests/qualification/`.

```text
out/build/qualification/doba_coverage --build out/build/coverage
  --output out/coverage --compiler clang++ --llvm-cov llvm-cov
  --llvm-profdata llvm-profdata
  --baseline tests/qualification/baselines/linux.json
```

Use one command line. Windows selects `baselines/windows.json` and can pass
`--openssl-root <directory>`. Both output and profiles must be fresh; a reused
build directory is allowed. An existing output directory is rejected, avoiding
accidental inclusion of stale profiles. Compiler/tool versions, commands,
source hashes and individual test logs accompany every report.

The coverage tool prints phase start/end times, progress for each executable
and a heartbeat every fifteen seconds while a subprocess runs. Configure,
build and CTest output is streamed to CI and written incrementally to logs.
LLVM JSON/LCOV exports remain separate files and are not printed to the console.

### Aggregation and denominator

This library is implemented in headers. Combining all executable mappings and
all profiles in one LLVM export can produce hash mismatches for unused inline
definitions and conflicting `main` functions. Such a report can omit code and
depend on executable order. It is not an acceptable baseline.

The private forced-include `profile_identity.h` is used only in this build.
In instrumented tests it sets the target's profile filename at startup. A separate
private probe exits before application main and writes a reset, zero-counter
profile. A private CMake definition namespaces files by executable target;
LLVM's module signature is cross-checked within that namespace. Module
signatures alone are not unique across executables. Namespace collisions fail
qualification. An executable with no flushed test profile receives only its
zero profile, never coverage credit
from another executable. No application server is started by this probe.

Each object is exported separately with its own indexed profile. Hash-mismatch
diagnostics fail qualification rather than being suppressed. The report takes
the union of LLVM LCOV executable source lines and JSON condition outcomes:

- A source line is covered when at least one measured instantiation hits it.
- A condition is identified by its source range and LLVM region kind. Its
  true and false outcomes are tracked independently across instantiations.
- Shared headers count once, not once per executable or template instance.
- Only `include/` contributes to these totals. Tests, examples, STL and OpenSSL
  do not inflate the production percentage.
- Headers without emitted mapping are listed in `unmapped_headers`. They are
  not silently declared covered or included as guessed executable lines.

These source-union metrics are not an average of LLVM per-object percentages,
an instantiation score or MC/DC. Per-object LLVM exports remain available for
inspection. A branch hit does not establish meaningful assertion coverage.
Killed processes may not flush profiles; the identity inventory makes lack of
observed profiles visible. Zero-profile counters are reset before writing.
The private CMake include and forced header never reach exported consumers.

The underlying instrumentation is documented in
[LLVM source-based coverage](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html).

The artifact contains:

| File | Evidence |
| --- | --- |
| `html/index.html` | Annotated production source and true/false outcomes. |
| `summary.md`, `summary.json` | Per-module counts, percentages, unmapped headers and uncovered outcomes. |
| `coverage.json` | Aggregated line and condition locations. |
| `profile-identities.json` | Binary/signature association and observed profile counts. |
| `object-*/` | Separate profiles and original LLVM JSON/LCOV exports, including zero-profile probes. |
| `source-sha256.json`, command/tool logs, `status.json` | Provenance and final qualification status. A failed test run remains failed even if a diagnostic report can be exported. |

### Regression gates

The per-platform baselines contain reviewed floors derived from measured
Release results, plus the measured header inventory. Each module must satisfy
its line and condition-outcome floor; removal of a previously measured header
or a new unmapped header also fails. A high parser percentage cannot hide lost
native coverage. Initial floors reserve one percentage point before rounding
down to an integer; the policy and exact floors are stored in each baseline.

Do not automatically regenerate baselines in CI or lower them to pass a PR.
Review the changed uncovered locations, compiler/platform differences and
actual tests before changing a floor. Adding executable code can legitimately
fail the gate and require new tests. A floor is a regression guard, not an
assertion that the remaining uncovered code is safe.

## Systematic campaigns

`doba_campaign` requires a Release build and validates its registered inventory.
It executes each selected case in an isolated process and records every
command, result and log. Default coverage includes:

| Family | Checks exercised |
| --- | --- |
| Engine | Deterministic 144-case state model, mixed ordering, RX reuse, cancellation, deadlines and admission limits. |
| Common ownership | Every registered cache, byte-storage and reader case, including failed loads, invalidation races, spill, moves and cleanup. |
| Linux | Every context case, partial sends/EAGAIN, source failure/budgets, descriptor exhaustion/partial startup, processing publication and cancellation probes. |
| Windows | IOCP completion before API return, partial startup, immediate submission failures, timer publication/cancellation and stale wake probes. |
| Winsock ownership | Three injected startup error codes, exact exception, no release after failed construction, successful retry and nested owner cleanup. |
| Native output | Default-window binary reader pipelines under one/four workers. |
| Optional TLS | Authenticated-record rejection, processing state combinations, native wire concurrency, incomplete handshakes and recovery. |

```text
out/build/asan/tests/qualification/doba_campaign --build out/build/asan
  --output out/campaign --rounds 2 --seed 104729 --tls
```

Omit `--tls` only for a deliberately TLS-disabled build. A missing required
group is an error, not a skip. Nonzero exit, assertion/sanitizer output,
timeout, empty selection or a skipped CTest group fails the campaign. The seed
controls **case order**, not OS thread scheduling. Controlled native probes
establish the interleavings they claim; arbitrary kernel schedules are not
reproducible merely from this seed. `results.json` records the actual order.

## CI budgets

| Trigger | Sanitizer campaigns | Decoder/chunked/engine fuzzing | Coverage |
| --- | --- | --- | --- |
| PR / push / merge queue / manual | Two rounds in existing Linux ASan/UBSan/TSan and Windows ASan/UBSan jobs. | 60 seconds per harness/OS plus canonical replay. | Complete instrumented Release suite on each OS, with module regression gates. |
| Nightly, 03:17 UTC | Twelve rounds in those jobs. | 600 seconds per harness/OS, with persisted evolving corpora. | Same gated per-platform measurement. |

Dedicated TLS jobs retain their existing five repetitions per selected TLS
group. Local qualification additionally runs the twelve-round campaign with
`--tls`; TLS-disabled sanitizer jobs do not silently claim that extra scope.

Nightly execution uses the default branch and requires the workflow to have
been merged there. Existing required-check names remain stable. Two new
coverage checks must be selected separately in repository rules if they are
to block merging. Release creation also depends on the coverage job. This
change does not modify repository permissions or branch rules.

`qualification_controls.cpp` checks the tools themselves: complementary branches,
order-independent union, duplicate source counting, unmapped headers, dropped
coverage, missing modules/headers, malformed inventory, CRLF exact-name
selection, sanitizer reports from exit-zero children, failed/empty selection,
timeouts and HTML escaping. The CTest group is mandatory in CI inventory.
The diagnostic group also validates native process argument quoting, large
stdout/stderr capture, stdin and environment isolation, SHA-256 known answers,
strict JSON parsing and malformed independent HTTP response controls.

Source filenames and source roots are both resolved before containment checks.
Regression fixtures replay JSON/LCOV collection, inventory and HTML generation
with noncanonical roots, including Windows 8.3 temporary-directory aliases.
Neighboring trees and test-only paths must remain excluded.

These campaigns extend the existing probes; they do not yet provide arbitrary
allocation-failure sweeps, complete native/TLS stateful fuzzing, exhaustive
schedules or a security certification. Remaining work is tracked in the
[security audit](../security-audit.md).

## Native tooling qualification, 2026-11-04

The thirteen former interpreter-based scripts have been replaced with C++.
CTest group names, Release execution, campaign selection controls and coverage
floors remain unchanged. TCP-only builds do not acquire an OpenSSL dependency
from these tools. The native TLS peers use OpenSSL independently of doba.

Local qualification, with TLS and examples enabled:

| Check | Linux | Windows |
| --- | --- | --- |
| Complete Release CTest suite | 21/21 groups | 23/23 groups |
| Two-round native fault campaign | 148 actions | 116 actions |
| Affected native runner groups, ASan | 7/7 | 7/7 |
| Affected native runner groups, UBSan | 7/7 | Not rerun for this migration |
| Fresh instrumented Release coverage gates | Passed | Passed |
| Replay of published LLVM exports with the C++ union | Identical | Identical |

The replay compares module counts, measured/unmapped inventories and uncovered
outcomes against the published baseline's exports. Fresh runs measure
8,169/8,585 lines and 4,730/5,340 outcomes on Linux; Windows measures
8,436/8,914 lines and 4,786/5,410 outcomes. The published release snapshot
retains its original measurement. Coverage floors were not lowered.

The native process checks include large stdout/stderr, exact quoted arguments,
stdin, environment isolation and exit-zero sanitizer reports. Linux example
shutdown preserves SIGTERM handling; Windows signal-based examples retain
their hidden private console. All 189 production header hashes are unchanged.
These are local results, not a new remote CI run.

## Reliability finding during qualification

The expanded Windows ASan campaign failed on round eight in the processing
publication probe. Its reply bytes and close checks passed; the fixture also
required a native timer packet from a one-millisecond deadline. Reconciliation
may legitimately find that deadline already due and post immediate processing
instead. That expectation depended on timing, not the transport contract.

Coverage was preserved and extended rather than deleting the timer obligation:

- Native output is tested with both future and already-due deadlines. The
  already-due variant must not create a native timer.
- A separate real IOCP consumer forces early timer-packet dequeue while the
  producer is still publishing; the exact retained owner/version is checked.
- The timer registry injects two consecutive post failures, including retry
  from the native callback, and must retain and return the exact registration.
- Stale-wake cancellation uses a controlled native timer notification against
  a far-future logical deadline, preserving actual socket disconnect/reconnect.
- Timer/context cleanup now keeps the referenced stopping flag alive even
  when a fixture exits early on a failed assertion.

The failed campaign and log remain in local evidence. A fresh campaign follows
the fixture correction; there is no retry-until-green policy or transport change.
