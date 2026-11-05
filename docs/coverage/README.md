<a name="coverage"></a>
<h1>
  <a href="../../resources/coverage/overview/h1-coverage.svg#gh-light-mode-only">
    <img src="../../resources/coverage/overview/h1-coverage.svg" alt="Coverage">
  </a>
  <a href="../../resources/coverage/overview/h1-coverage-dark.svg#gh-dark-mode-only">
    <img src="../../resources/coverage/overview/h1-coverage-dark.svg" alt="Coverage">
  </a>
</h1>

[Back to doba](../../README.md#fast-is-nice-correct-is-non-negotiable)

**doba 0.1.0, measured rather than guessed.** The latest local coverage
measurements are from **2026-11-05 for Linux** and **2026-11-04 for Windows**,
with optimizations, TLS and examples enabled. The version is declared in
[version.h](../../include/version.h). These measurements qualify the tested
source snapshot; they do not assert that a matching release tag was published.
The Linux refresh passes all existing coverage gates. The reported remote
Linux coverage failure preceded this correction; a successful remote run
of the corrected snapshot has not been verified.

[Results](#results) / [By module](#by-module) / [Test families](#test-families) /
[Method](#method) / [CI safeguards](#ci-safeguards) /
[Limits](#limits) / [Release history](#release-history)

<a name="results"></a>
## The measured result

| Platform | Measured on | Executable lines | Condition outcomes | Mapped headers | Unmapped headers |
| --- | --- | --- | --- | --- | --- |
| Linux | 2026-11-05 | **8,271 / 8,691 (95.17%)** | **4,751 / 5,346 (88.87%)** | 129 | 60 |
| Windows | 2026-11-04 | **8,436 / 8,914 (94.64%)** | **4,788 / 5,410 (88.50%)** | 136 | 53 |

Only production headers under `include/` contribute. Tests, examples, STL and
OpenSSL implementation code are excluded from these percentages. Each platform
has its own denominator. Unmapped headers remain explicit; they are not counted
as covered, nor assigned guessed executable lines.

The [updated 0.1.0 data snapshot](releases/0.1.0-2026-11-05.json) contains
independent measurement dates and source hashes for each platform. It retains
the historical Windows measurement rather than attributing it to the refreshed
Linux source. The [initial snapshot](releases/0.1.0.json) remains unchanged.
Both snapshots contain exact module counts,
per-header line and condition counts, the full unmapped inventory, uncovered
condition locations, toolchain details and production-header SHA-256 hashes.

<a name="by-module"></a>
## By module

| Module | Linux lines | Linux outcomes | Windows lines | Windows outcomes |
| --- | --- | --- | --- | --- |
| common | 89.88% | 81.96% | 88.67% | 78.95% |
| http-common | 97.06% | 93.83% | 97.06% | 93.88% |
| http-engine | 97.61% | 93.08% | 97.61% | 93.08% |
| http-messages | 91.83% | 83.33% | 91.87% | 83.33% |
| http-parser | 95.91% | 91.31% | 95.90% | 91.41% |
| network | no emitted mapping | no emitted mapping | 100.00% | 100.00% |
| public-contracts | 100.00% | 100.00% | 100.00% | 100.00% |
| static-files | 96.69% | 83.62% | 95.54% | 79.53% |
| tcp | 94.19% | 79.52% | 91.78% | 83.88% |
| tls | 99.21% | 79.78% | 99.21% | 79.78% |

`common` covers storage, readers, cache and task utilities. `http-parser`
includes decoding, body framing and HTTP/1.1 header rules. `http-engine`
includes the common engine and synchronous/asynchronous request processors.
`tcp` includes the active native backend and shared connection machinery;
`tls` measures doba's adapter and configuration code. `public-contracts`
groups public wrappers and protocol/transport contracts with emitted mapping.
Linux's network environment has no emitted runtime mapping in this snapshot.

The Linux refresh covers explicit cache invalidation of all representations,
retained response snapshots, invalid static-file policies and escaped directory
redirects. The formatting pass changed executable-line counts; the regression
floors remain unchanged.

<a name="test-families"></a>
## What the tests exercise

| Family | Covered behavior |
| --- | --- |
| Common ownership | Binary readers/writers, storage spill and cleanup, cache limits, shared loads, invalidation races, exceptions and coroutine frame ownership. |
| HTTP parsing | Fragmentation, conflicting framing, malformed fields and targets, exact limits, caller-buffer reuse and request ownership. |
| Routing and authentication | Route precedence, typed parameters, suspended parameter views, Basic/Bearer credential parsing, challenges and header-injection rejection. |
| HTTP output | Serialization, status/field/trailer validation, exact binary framing and independent wire-oracle negative controls. |
| HTTP execution | Synchronous and coroutine handlers, mixed ordered responses, bounded admission, deadlines, cancellation and source lifetimes. |
| Static files | Path containment, metadata and explicit cache invalidation, retained snapshots, variants, ranges, conditional responses, invalid policies and escaped redirects. |
| Linux TCP | epoll processing, partial sends, EAGAIN, prefix/source bounds, descriptor exhaustion, startup recovery and scheduling cancellation. |
| Windows TCP | IOCP publication/completion ordering, send/receive failures, timer retries, stale wakes, shutdown and connection cleanup. |
| Winsock lifecycle | Three injected startup errors, exact rejection, no cleanup after failed construction, successful retry and nested owner cleanup. |
| TLS | Verified TLS 1.2/1.3, bounded BIO progress, KeyUpdate, prefix/source, authenticated-record corruption and mixed native HTTPS processing. |
| Test infrastructure | Negative oracle controls, assertion/exception propagation, watchdogs, sanitizer diagnostics from exit-zero children, inventory and coverage regression gates. |
| Package and examples | Source/installed consumers, TLS dependency boundaries and executable examples; configured in the broader CI matrix. |

The instrumented suite results below follow each platform's measurement date.
The sanitizer campaigns and repeated fault groups are historical results from
2026-11-04; they were not repeated for the Linux coverage refresh. The updated
Windows MSVC Release suite also passes 23/23 groups, without a new Windows
instrumented measurement. Configured CI jobs and source-level test declarations
are separate quantities.

| Local validation | Linux | Windows |
| --- | --- | --- |
| Complete instrumented Release CTest suite | **21/21 groups passed** | **23/23 groups passed** |
| Twelve-round campaign, including TLS | **888 actions passed under TSan** | **696 actions passed under ASan** |
| Corrected IOCP processing group | Not applicable | **200 repetitions under ASan and 200 under UBSan passed** |
| Winsock fault group | Not applicable | **12 repetitions under UBSan passed**, also included in the ASan campaign |

The campaign exposed a timing-dependent Windows test premise. The corrected
fixtures explicitly force timer publication, retry and cancellation conditions,
while retaining the legitimate already-due processing path. A fresh campaign
passed after that correction; the failure was retained, not hidden by an
automatic retry. Production transport code was unchanged.

<a name="method"></a>
## How the measurement works

The isolated Clang Release builds use LLVM source instrumentation with atomic
counter updates. Profiles are namespaced by executable target and exported
separately. This matters for a header-only library: mixing executable mappings
can produce mismatched inline-function records and misleading totals.
All final per-object exports completed without LLVM diagnostics.

The report forms a union of executable source lines and condition locations.
A line is covered when a measured instantiation executes it; true and false
condition outcomes are tracked independently by source range. Shared headers
count once. The figures are not averages of per-executable percentages and
do not measure MC/DC or every possible template instantiation.

An executable with no flushed test profile receives zero coverage credit.
Unmapped headers include platform alternatives, declaration-only headers and
uninstantiated code. The data snapshot preserves them for review.

| Measurement environment | Linux | Windows |
| --- | --- | --- |
| Toolchain | Clang/LLVM **18.1.3** | Clang/LLVM **22.1.8** |
| Host | WSL Ubuntu, Linux **6.6.87.2** | Windows 11 **10.0.26300** |
| Configuration | Release, TLS and examples enabled | Release, TLS and examples enabled |
| Profile association | 106 observed profiles / 62 executable targets | 79 observed profiles / 57 executable targets |

Instrumented builds are for qualification. Performance measurements use their
own builds. Reproduction commands, report layout and technical details live in
[Release qualification](../../tests/qualification/README.md).

<a name="ci-safeguards"></a>
## What CI requires

| Gate | Normal runs | Nightly runs |
| --- | --- | --- |
| Production coverage | Full instrumented Release suite on Linux and Windows; per-module floors and mapped/unmapped header checks. | Same measurement and gates. |
| Fault and ownership campaigns | Two rounds in the existing Linux ASan/UBSan/TSan and Windows ASan/UBSan jobs. | Twelve rounds in those jobs. |
| Decoder, chunked and engine fuzzing | Canonical replay plus 60 seconds per harness/OS under ASan/UBSan. | 600 seconds per harness/OS; evolving corpora retained. |
| Targeted concurrency | Existing 200 engine repetitions and 20 native repetitions in their configured sanitizer jobs. | Retained alongside the extended campaigns. |
| TLS and mutation checks | Dedicated TLS repetitions and eight selected mutation detectors, plus package/example checks. | Retained in the scheduled matrix. |

Campaign seeds reproduce case order; they do not reproduce arbitrary OS thread
schedules. Controlled native probes establish the interleavings they claim.
TLS-disabled jobs do not claim TLS execution; dedicated TLS jobs retain their
five repetitions per selected group. The local twelve-round measurements above
explicitly include TLS.

Coverage floors are reviewed per platform, derived from measurements with a
one-percentage-point reserve before rounding down. A module regression,
previously measured header disappearing or a new unmapped header fails the
gate. CI does not silently regenerate or lower the
[Linux](../../tests/qualification/baselines/linux.json) and
[Windows](../../tests/qualification/baselines/windows.json) floors.

The [workflow](../../.github/workflows/ci.yml) archives HTML/JSON, source hashes,
tool versions and execution logs. Existing check names remain stable; two new
coverage checks also gate release creation. Repository rules must require those
new checks separately after their first remote run. These are configured
safeguards, not a claim that a new remote workflow has already passed.

<a name="limits"></a>
## What these figures cannot establish

Coverage shows execution, not that every assertion is sufficient or every
schedule is safe. Uncovered outcomes remain in common ownership/error handling,
native TCP cleanup/scheduling and TLS error paths. Exhaustive allocation-failure
sweeps, native/TLS stateful fuzzing and filesystem/cache operation fuzzing remain
work identified by the [security audit](../../tests/security-audit.md).

Passing sanitizers and finite campaigns does not prove absence of races,
corruption or vulnerabilities. This reference records the evidence and its
scope; it is not a security certification.

<a name="release-history"></a>
## Keep the reference current

| Source version | Measurement date | Evidence status | Archived data |
| --- | --- | --- | --- |
| **0.1.0 refresh** | **Linux: 2026-11-05; Windows: 2026-11-04** | Local coverage gates passed on Linux; historical Windows coverage retained | [Updated snapshot](releases/0.1.0-2026-11-05.json) |
| **0.1.0 initial** | **2026-11-04** | Initial local Release qualification | [Initial snapshot](releases/0.1.0.json) |

For each release, measure the final source with the documented toolchains,
review the uncovered locations and record the CI/artifact provenance. Add a new
versioned data snapshot, update this report and the README's quality card from
the same results, and retain older snapshots in the history table. Review any
change to regression floors separately; publishing results must not lower them
automatically. Keep local and remote validation statuses explicit.
