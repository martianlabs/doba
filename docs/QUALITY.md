# Quality

[Index](HANDOFF.md)

**Correctness is a requirement. Confidence must be earned through verification.**

Doba's quality rules apply to implementation, tests, packaging, and releases.
A change must preserve protocol behavior, ownership, and the contracts exposed
to applications. Passing checks provide evidence for the paths and
configurations exercised.

## Make correctness explicit

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

## Treat ownership and failure as part of the contract

- Use RAII for owned resources. Every view must have a valid backing lifetime,
  including across moves, buffer reuse, asynchronous work, and cancellation.
- Treat network input as sized bytes. Never assume null termination.
- Review error paths alongside success paths: failed startup, partial I/O,
  disconnects, serialization failure, and shutdown must have defined cleanup.
- Keep concurrency rules explicit: which thread owns state, how it is
  synchronized, and when outstanding work may still access it.
- Document resource limits and failure behavior. A local buffer bound must
  not be presented as a complete connection or request resource policy.
- Preserve equivalent externally observable behavior on Windows and Linux.

## Verify behavior at the right boundary

Use focused unit tests for parsing, framing, value types, and object
contracts. Use real-socket integration tests for behavior that depends on the
transport: fragmentation, pipelining, response ordering, concurrent clients,
cancellation, disconnects, and restart.

Changes to shared transport behavior require equivalent coverage in IOCP and
epoll. A fake transport can isolate a failure path; it does not replace
real-socket verification of network behavior.

Run focused regressions first, then the affected suites. The complete CI
matrix validates the supported build configurations.

## Enforce the CI gates

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

## Protect consumers

- Preserve public signatures, ownership, exception guarantees, and observable
  behavior unless a compatibility change is intentional and documented.
- Keep internal warning and sanitizer flags out of the installed target's
  interface.
- Validate packaging from an isolated consumer, not just the library's own
  build.
- Keep source, examples, and documentation aligned with the supported API.

## Require evidence for performance claims

Changes justified by performance need reproducible measurements.
Record the revision, compiler and flags, workload, environment, and repeated
samples. Compare throughput and latency, and measure memory or allocations
when those are the claimed benefit.

Do not trade correctness for an assumed speedup. Do not describe an
optimization as an improvement when its measured difference is within noise.
Publish performance and compliance claims only at the scope supported by
the evidence.

## Release only a traceable revision

The release workflow runs from `main` after the compiler, sanitizer, minimum
CMake, and consumer checks succeed. It derives the version from
`include/version.h` and rejects an existing tag that points elsewhere.

Release decisions must refer to the actual revision and its CI results.
A previous successful run does not validate later changes. Report which
checks ran and any verification limits; never claim a test passed without
execution.

## Keep documentation and changes reviewable

All repository documentation is written in English. Text uses ASCII,
UTF-8 without BOM, CRLF in the working tree, and a final newline.
Changes preserve local code style and avoid unrelated formatting.

For documentation changes, verify technical claims, relative links, and
formatting. For implementation changes, also report the relevant tests and
results. [Development](DEVELOPMENT.md) provides the build commands and
conventions; [Backlog](BACKLOG.md) records outstanding work and release scope.
