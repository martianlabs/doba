<a name="http-regression-detection"></a>
<h1>
  <a href="../../resources/tests/mutation/h1-http-regression-detection.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/mutation/h1-http-regression-detection-narrow.svg">
      <img src="../../resources/tests/mutation/h1-http-regression-detection.svg" alt="Would the tests catch it?">
    </picture>
  </a>
  <a href="../../resources/tests/mutation/h1-http-regression-detection-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/mutation/h1-http-regression-detection-narrow-dark.svg">
      <img src="../../resources/tests/mutation/h1-http-regression-detection-dark.svg" alt="Would the tests catch it?">
    </picture>
  </a>
</h1>

Let's break something on purpose. We plant eight bugs in private header copies
and ask the existing tests to catch them. Your working headers stay untouched.

The original must pass. The broken copy must compile and fail at the expected
assertion. A compiler error, crash, timeout or sanitizer report doesn't count
as catching the planted bug.

<a name="what-gets-caught"></a>
<h2>
  <a href="../../resources/tests/mutation/h2-what-gets-caught.svg#gh-light-mode-only">
    <img src="../../resources/tests/mutation/h2-what-gets-caught.svg" alt="What gets caught">
  </a>
  <a href="../../resources/tests/mutation/h2-what-gets-caught-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/mutation/h2-what-gets-caught-dark.svg" alt="What gets caught">
  </a>
</h2>

| Planted bug | What should complain |
| --- | --- |
| Ignore the body limit | One extra byte is rejected. |
| Reorder responses | A later reply waits for the earlier one. |
| Forget terminal rejection | Later input cannot reach a handler. |
| Remove admission checks | A full connection budget refuses another slot. |
| Allow another reader | A second terminal reader is rejected. |
| Forget sent bytes in an active batch | Partial sends still count toward the TX limit. |
| Ignore queued response bytes | Overflow closes the connection. |
| Ignore request positions | Work beyond the window is refused. |

The [HTTP detectors](../unit/protocol/http/v11/regression_detection_tests.cpp)
and policy detectors also run in the unit suite. Their files are listed in
[CMakeLists.txt](CMakeLists.txt).

From the repo root, with Clang and Ninja ready (a developer shell on Windows):

```sh
cmake -P tests/mutation/run_detection.cmake
```

Without Git, pass `-DDOBA_MUTATION_SOURCE_DESCRIPTION=<description>` before
`-P`. The campaign records source hashes and checks that the originals did
not change. Eight caught bugs are useful; they're not a score for the whole library.
