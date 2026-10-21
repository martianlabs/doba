<a name="test-helpers"></a>
<h1>
  <a href="../../resources/tests/helpers/h1-test-helpers.svg#gh-light-mode-only">
    <img src="../../resources/tests/helpers/h1-test-helpers.svg" alt="Test helpers">
  </a>
  <a href="../../resources/tests/helpers/h1-test-helpers-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/helpers/h1-test-helpers-dark.svg" alt="Test helpers">
  </a>
</h1>

Shared plumbing for the unit and integration suites. The tests get to be
awkward; their housekeeping shouldn't be.

[concurrency.h](concurrency.h) provides controlled waits, coroutine events,
and automatic cleanup. [test_watchdog.h](test_watchdog.h) enforces deadlines
and names the test that got stuck, including during cleanup.
[platform.h](platform.h) selects native helper backends so Linux/Windows
branches stay out of shared test code.
