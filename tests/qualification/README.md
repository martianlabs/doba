<a name="release-qualification"></a>
<h1>
  <a href="../../resources/tests/qualification/h1-release-qualification.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/h1-release-qualification.svg" alt="Before we ship">
  </a>
  <a href="../../resources/tests/qualification/h1-release-qualification-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/h1-release-qualification-dark.svg" alt="Before we ship">
  </a>
</h1>

Run the suite, look at what it missed, then make failures happen on purpose.
These tools help with the last two steps.

Start with the [coverage snapshot](BASELINE.md) for numbers,
[policy checks](POLICIES.md) for limits, or the [test review](../security-audit.md)
for things we still want to push harder.

<a name="source-coverage"></a>
<h2>
  <a href="../../resources/tests/qualification/h2-source-coverage.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/h2-source-coverage.svg" alt="Where the tests reach">
  </a>
  <a href="../../resources/tests/qualification/h2-source-coverage-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/h2-source-coverage-dark.svg" alt="Where the tests reach">
  </a>
</h2>

`doba_coverage` makes a separate Clang Release build with TLS and examples,
runs all registered tests and collects LLVM coverage. It doesn't add flags to
installed targets. Don't use this instrumented build for benchmarks.

You need matching `clang++`, `llvm-profdata` and `llvm-cov`, plus Ninja
and OpenSSL. Windows also needs a developer shell and OpenSSL on PATH.
CI uses LLVM 18 on Linux and pins LLVM 22.1.8 on Windows.

Build the tools (ordinary root test builds already include them):

```sh
cmake -S tests/qualification -B out/build/qualification -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build out/build/qualification --parallel 2
out/build/qualification/doba_coverage --build out/build/coverage --output out/coverage --compiler clang++ --llvm-cov llvm-cov --llvm-profdata llvm-profdata --baseline tests/qualification/baselines/linux.json
```

On Windows, add `.exe` to tool names, use `baselines/windows.json` and pass
`--openssl-root <directory>` if needed. Pick a fresh output directory; stale
profiles must not get mixed into a new result. Reusing the build directory is fine.
The tool streams progress and keeps commands, versions, hashes and test logs.

<a name="aggregation-and-denominator"></a>

### <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> How we count

doba lives in headers, so the same code can appear in many executables.
We export each executable with its own profiles, then combine the results:

- A line counts once, even if many template instances hit it.
- Each condition's true and false outcomes count separately.
- Only `include/` counts. Tests, STL and OpenSSL don't pad the score.
- Headers with no emitted mapping stay in `unmapped_headers`, outside the percentage.

Executables without a test profile get a zero-counter profile. Mismatched
profiles fail the run. The [collector](coverage.cpp) and
[LLVM's coverage docs](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html)
have the implementation details.

Open `html/index.html` to browse missed code. `summary.md` and
`summary.json` give module totals; the JSON/LCOV exports, profile identities
and source hashes let you trace them back. A failed test still means a failed
run, even if the tool managed to write a report.

<a name="regression-gates"></a>

### <a href="../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Keep coverage from slipping

The reviewed [Linux](baselines/linux.json) and [Windows](baselines/windows.json)
baselines set a floor for each module. Losing a measured header or adding an
unmapped one also fails the check. Good parser coverage can't hide lost TCP tests.

Don't lower a floor just to turn a PR green. Look at the missed code first;
new code may need new tests. The JSON files record the measured values and
rounding rule.

<a name="systematic-campaigns"></a>
<h2>
  <a href="../../resources/tests/qualification/h2-systematic-campaigns.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/h2-systematic-campaigns.svg" alt="Run it again">
  </a>
  <a href="../../resources/tests/qualification/h2-systematic-campaigns-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/h2-systematic-campaigns-dark.svg" alt="Run it again">
  </a>
</h2>

`doba_campaign` runs selected Release tests in separate processes and keeps
every command and result. It covers engine ordering, common storage/ownership,
native failures, HTTP/TCP policies and binary output. Add `--tls` for TLS checks.

```sh
out/build/release/tests/qualification/doba_campaign --build out/build/release --output out/campaign --rounds 2 --seed 104729
```

Use a built Release tree and a fresh output directory. Add `.exe` on Windows.
Only use `--tls` when that build has TLS enabled.

Missing or skipped groups, empty selections, timeouts, assertions and sanitizer
reports all fail the campaign. The seed fixes **test order**, not OS thread
scheduling. `results.json` records what actually ran.

<a name="ci-budgets"></a>
<h2>
  <a href="../../resources/tests/qualification/h2-ci-budgets.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/h2-ci-budgets.svg" alt="What CI runs">
  </a>
  <a href="../../resources/tests/qualification/h2-ci-budgets-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/h2-ci-budgets-dark.svg" alt="What CI runs">
  </a>
</h2>

| When | Sanitizer campaigns | Fuzzing, per harness/OS | Coverage |
| --- | --- | --- | --- |
| PR, push, merge queue or manual | 2 rounds | Seed replay + 60 seconds | Full Release suite, both OSes. |
| Nightly, 03:17 UTC | 12 rounds | Seed replay + 600 seconds | Same module floors. |

Linux runs ASan/UBSan/TSan; Windows runs ASan/UBSan. Dedicated TLS jobs repeat
their selected groups five times. A TLS-disabled job doesn't cover TLS by magic.
Nightly jobs run from the default branch and keep the growing fuzz corpora.

<a name="native-tooling-qualification-2026-11-04"></a>
<h2>
  <a href="../../resources/tests/qualification/h2-native-tooling-qualification-2026-11-04.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/h2-native-tooling-qualification-2026-11-04.svg" alt="The tools get tests too">
  </a>
  <a href="../../resources/tests/qualification/h2-native-tooling-qualification-2026-11-04-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/h2-native-tooling-qualification-2026-11-04-dark.svg" alt="The tools get tests too">
  </a>
</h2>

The runners and tools are C++20; no interpreter is needed. Their own
[checks](qualification_controls.cpp) try missing groups, dropped coverage,
bad profiles, timeouts and sanitizer reports from children that exit zero.
[Process checks](../helpers/tool_controls.cpp) also cover quoted arguments,
stdin, large output, JSON, hashes and response parsing.

The switch from scripts to C++ was checked locally on 2026-11-04: Release
suites, coverage floors and replayed LLVM totals passed on both OSes.
ASan runner checks passed 7/7 on each; Linux UBSan passed 7/7. Windows UBSan
wasn't rerun for that migration. Logs are in `out/reliability/2026-11-04`.

<a name="reliability-finding-during-qualification"></a>
<h2>
  <a href="../../resources/tests/qualification/h2-reliability-finding-during-qualification.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/h2-reliability-finding-during-qualification.svg" alt="A timer test that cried wolf">
  </a>
  <a href="../../resources/tests/qualification/h2-reliability-finding-during-qualification-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/h2-reliability-finding-during-qualification-dark.svg" alt="A timer test that cried wolf">
  </a>
</h2>

A Windows ASan campaign failed on round eight: the reply was right, but the
test demanded a timer packet for a one-millisecond deadline. That deadline
could already be due, making an immediate turn perfectly valid.

We fixed the expectation and kept separate controlled checks for future timers,
already-due work, early IOCP dequeue, two failed timer posts and stale wakes.
The failed log stays with the later runs. The [snapshot](BASELINE.md) records
the results; [remaining work](../security-audit.md#prioritized-remaining-work)
includes wider fault injection and longer stress tests.
