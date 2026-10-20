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

This bounded campaign checks that three functional assertions detect
specific defects. It compiles the original tests first, then builds each mutant
against an independent copy of the production headers. Your working headers
are never edited.

Each selected original test must pass exactly once. Each mutant must
compile, exit with the runner's assertion-failure code, and report the
planned assertion. A compiler error, timeout, crash, or sanitizer diagnostic
is a failed campaign, never a successful detection.

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

The checks also verify that the original headers and detector tests retain
their hashes after the campaign. The mutations stay in private copies.
This checks three selected regressions; it is not a mutation score for the
whole library.
