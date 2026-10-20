<a name="http-regression-detection"></a>
<h1>
  <a href="../../resources/tests/mutation/h1-http-regression-detection.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/mutation/h1-http-regression-detection-narrow.svg">
      <img src="../../resources/tests/mutation/h1-http-regression-detection.svg" alt="HTTP regression detection">
    </picture>
  </a>
  <a href="../../resources/tests/mutation/h1-http-regression-detection-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/mutation/h1-http-regression-detection-narrow-dark.svg">
      <img src="../../resources/tests/mutation/h1-http-regression-detection-dark.svg" alt="HTTP regression detection">
    </picture>
  </a>
</h1>

This bounded Linux campaign checks that three functional assertions detect
specific defects. It compiles the original tests first, then builds each mutant
against an independent copy of the production headers. Your working headers
are never edited.

<a name="build-and-run"></a>
<h2>
  <a href="../../resources/tests/mutation/h2-build-and-run.svg#gh-light-mode-only">
    <img src="../../resources/tests/mutation/h2-build-and-run.svg" alt="Build and run">
  </a>
  <a href="../../resources/tests/mutation/h2-build-and-run-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/mutation/h2-build-and-run-dark.svg" alt="Build and run">
  </a>
</h2>

From the repository root on Linux or WSL with Clang and its sanitizer runtime:

```sh
cmake -DDOBA_MUTATION_OUTPUT=out/mutations -P tests/mutation/run_detection.cmake
```

All executables use Release flags (`-O3 -DNDEBUG`), symbols, strict warnings,
ASan and UBSan. Each selected original test must pass exactly once. Each mutant
must compile, exit with the runner's assertion-failure code and report the
planned assertion. A compiler error, timeout or sanitizer diagnostic fails the
campaign instead of counting as successful detection.

<a name="what-gets-caught"></a>
<h2>
  <a href="../../resources/tests/mutation/h2-what-gets-caught.svg#gh-light-mode-only">
    <img src="../../resources/tests/mutation/h2-what-gets-caught.svg" alt="What gets caught">
  </a>
  <a href="../../resources/tests/mutation/h2-what-gets-caught-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/mutation/h2-what-gets-caught-dark.svg" alt="What gets caught">
  </a>
</h2>

| Mutant | Regression | Expected assertion |
| --- | --- | --- |
| body-limit | Disable the configured Content-Length body budget | A body one byte above the limit is rejected |
| response-order | Send a completed successor before its pending predecessor | No output is sent while the predecessor is suspended |
| rejection-successor | Notify transport closure but omit the engine's terminal-state update | A later receive cannot dispatch a successor |

The detector tests live in
[regression_detection_tests.cpp](../unit/protocol/http/v11/regression_detection_tests.cpp)
and also run in the ordinary unit suite. They include accepted-boundary,
successful-order and terminal-close checks on the original code.

<a name="diagnostics"></a>
<h2>
  <a href="../../resources/tests/mutation/h2-diagnostics.svg#gh-light-mode-only">
    <img src="../../resources/tests/mutation/h2-diagnostics.svg" alt="Diagnostics">
  </a>
  <a href="../../resources/tests/mutation/h2-diagnostics-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/mutation/h2-diagnostics-dark.svg" alt="Diagnostics">
  </a>
</h2>

Each output directory preserves original header/test hashes, the
Git revision, compiler, CMake configuration, compile commands, each mutation's
before/after replacement, original and mutant test logs, and `results.txt`.
The CMake driver verifies that original
headers and detector tests still have the same hashes before returning.
CI retains these diagnostics for 14 days and gates release on all three kills.
This is a demonstration for three selected regressions, not a mutation score
for the whole library.

Only repository C++ sources and CMake build and run the checks. No scripting
runtime is needed. To select a compiler, pass
`-DDOBA_MUTATION_COMPILER=/usr/bin/clang++` before `-P`.
