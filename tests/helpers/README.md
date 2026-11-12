<a name="test-helpers"></a>
<h1>
  <a href="../../resources/tests/helpers/h1-test-helpers.svg#gh-light-mode-only">
    <img src="../../resources/tests/helpers/h1-test-helpers.svg" alt="Test helpers">
  </a>
  <a href="../../resources/tests/helpers/h1-test-helpers-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/helpers/h1-test-helpers-dark.svg" alt="Test helpers">
  </a>
</h1>

The tests can be awkward. Their housekeeping shouldn't be.

- [concurrency.h](concurrency.h): hold work at a known point, then release it
  safely during cleanup.
- [test_watchdog.h](test_watchdog.h): stop a stuck test and tell us which one.
- [platform.h](platform.h): pick the native helper so shared tests stay readable.
