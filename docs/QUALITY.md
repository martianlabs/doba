<a name="quality"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/quality/h1-quality-dark.svg">
    <img src="../resources/docs/quality/h1-quality.svg" alt="Quality">
  </picture>
</h1>

[Index](HANDOFF.md)

**Correctness is a requirement. Confidence must be earned through verification.**

Doba's quality rules apply to implementation, tests, packaging, and releases.
A change must preserve protocol behavior, ownership, and the contracts exposed
to applications. Passing checks provide evidence for the paths and
configurations exercised.

<a name="make-correctness-explicit"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/quality/h2-make-correctness-explicit-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/quality/h2-make-correctness-explicit-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/quality/h2-make-correctness-explicit-dark.svg">
    <img src="../resources/docs/quality/h2-make-correctness-explicit.svg" alt="Make correctness explicit">
  </picture>
</h2>

- Base protocol behavior on the applicable RFC section and the documented
  contract. Keep syntax validation separate from semantic decisions.
- Reproduce a defect before correcting it. Add a focused regression that
  fails before the fix and passes after it whenever the test infrastructure
  supports the case.
- Include the nearest meaningful valid, invalid, and boundary cases.
- Fix the cause with the smallest relevant change. Preserve unrelated
  behavior and existing public contracts.
- Never weaken assertions, silently skip failures, or broaden accepted input
  merely to obtain a passing result.

<a name="treat-ownership-and-failure-as-part-of-the-contract"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/quality/h2-treat-ownership-and-failure-as-part-of-the-contract-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/quality/h2-treat-ownership-and-failure-as-part-of-the-contract-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/quality/h2-treat-ownership-and-failure-as-part-of-the-contract-dark.svg">
    <img src="../resources/docs/quality/h2-treat-ownership-and-failure-as-part-of-the-contract.svg" alt="Treat ownership and failure as part of the contract">
  </picture>
</h2>

- Use RAII for owned resources. Every view must have a valid backing lifetime,
  including across moves, buffer reuse, pending I/O, and connection closure.
- Treat network input as sized bytes. Never assume null termination.
- Review error paths alongside success paths: failed startup, partial I/O,
  disconnects, serialization failure, and shutdown must have defined cleanup.
- Keep concurrency rules explicit: which thread owns state, how it is
  synchronized, and when outstanding work may still access it.
- Document resource limits and failure behavior. A local buffer bound must
  not be presented as a complete connection or request resource policy.
- Preserve equivalent externally observable behavior on Windows and Linux.

<a name="verify-behavior-at-the-right-boundary"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/quality/h2-verify-behavior-at-the-right-boundary-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/quality/h2-verify-behavior-at-the-right-boundary-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/quality/h2-verify-behavior-at-the-right-boundary-dark.svg">
    <img src="../resources/docs/quality/h2-verify-behavior-at-the-right-boundary.svg" alt="Verify behavior at the right boundary">
  </picture>
</h2>

Use focused unit tests for parsing, framing, value types, and object
contracts. Use real-socket integration tests for behavior that depends on the
transport: fragmentation, pipelining, response ordering, concurrent clients,
cancellation, disconnects, and restart.

Changes to shared transport behavior require equivalent coverage in IOCP and
epoll. A fake transport can isolate a failure path; it does not replace
real-socket verification of network behavior.

Run focused regressions first, then the affected suites. The complete CI
matrix validates the supported build configurations.

<a name="enforce-the-ci-gates"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/quality/h2-enforce-the-ci-gates-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/quality/h2-enforce-the-ci-gates-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/quality/h2-enforce-the-ci-gates-dark.svg">
    <img src="../resources/docs/quality/h2-enforce-the-ci-gates.svg" alt="Enforce the CI gates">
  </picture>
</h2>

The [CI workflow](../.github/workflows/ci.yml) defines the executable checks:

| Gate | Required validation |
| --- | --- |
| Compiler and configuration coverage | GCC and Clang on Linux; MSVC on Windows; Debug and Release. |
| Strict warnings | `-Wall -Wextra -Wpedantic -Werror` or `/W4 /WX` for the project's builds. |
| Memory safety checks | Unit and integration suites under Clang AddressSanitizer, with leak detection. |
| Undefined behavior checks | Unit and integration suites under Clang UndefinedBehaviorSanitizer. |
| Data race checks | Unit and integration suites under Clang ThreadSanitizer. |
| Build compatibility | Configure, build, and test with CMake 3.20.6. |
| Installed package | Build isolated Debug and Release consumers through `find_package`. |
| Source integration | Build an `add_subdirectory` consumer without importing internal tests or examples. |

Every gate must pass for a release. Investigate sanitizer findings as defects;
do not suppress them merely to make the pipeline pass. Preserve strict warning
settings instead of hiding diagnostics.

Sanitizers check executed paths. They complement regression tests and code
review; they do not establish that unexecuted paths are safe.

<a name="protect-consumers"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/quality/h2-protect-consumers-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/quality/h2-protect-consumers-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/quality/h2-protect-consumers-dark.svg">
    <img src="../resources/docs/quality/h2-protect-consumers.svg" alt="Protect consumers">
  </picture>
</h2>

- Preserve public signatures, ownership, exception guarantees, and observable
  behavior unless a compatibility change is intentional and documented.
- Keep internal warning and sanitizer flags out of the installed target's
  interface.
- Validate packaging from an isolated consumer, not just the library's own
  build.
- Keep source, examples, and documentation aligned with the supported API.

<a name="require-evidence-for-performance-claims"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/quality/h2-require-evidence-for-performance-claims-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/quality/h2-require-evidence-for-performance-claims-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/quality/h2-require-evidence-for-performance-claims-dark.svg">
    <img src="../resources/docs/quality/h2-require-evidence-for-performance-claims.svg" alt="Require evidence for performance claims">
  </picture>
</h2>

Changes justified by performance need reproducible measurements.
Record the revision, compiler and flags, workload, environment, and repeated
samples. Compare throughput and latency, and measure memory or allocations
when those are the claimed benefit.

Do not trade correctness for an assumed speedup. Do not describe an
optimization as an improvement when its measured difference is within noise.
Publish performance and compliance claims only at the scope supported by
the evidence.

<a name="release-only-a-traceable-revision"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/quality/h2-release-only-a-traceable-revision-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/quality/h2-release-only-a-traceable-revision-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/quality/h2-release-only-a-traceable-revision-dark.svg">
    <img src="../resources/docs/quality/h2-release-only-a-traceable-revision.svg" alt="Release only a traceable revision">
  </picture>
</h2>

The release workflow runs from `main` after the compiler, sanitizer, minimum
CMake, and consumer checks succeed. It derives the version from
`include/version.h` and rejects an existing tag that points elsewhere.

Release decisions must refer to the actual revision and its CI results.
A previous successful run does not validate later changes. Report which
checks ran and any verification limits; never claim a test passed without
execution.

<a name="keep-documentation-and-changes-reviewable"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/quality/h2-keep-documentation-and-changes-reviewable-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/quality/h2-keep-documentation-and-changes-reviewable-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/quality/h2-keep-documentation-and-changes-reviewable-dark.svg">
    <img src="../resources/docs/quality/h2-keep-documentation-and-changes-reviewable.svg" alt="Keep documentation and changes reviewable">
  </picture>
</h2>

All repository documentation is written in English. Text uses ASCII,
UTF-8 without BOM, CRLF in the working tree, and a final newline.
Changes preserve local code style and avoid unrelated formatting.

For documentation changes, verify technical claims, relative links, and
formatting. For implementation changes, also report the relevant tests and
results. [Development](DEVELOPMENT.md) provides the build commands and
conventions; [Backlog](BACKLOG.md) records outstanding work and release scope.
