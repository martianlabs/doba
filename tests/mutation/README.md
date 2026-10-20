# HTTP regression detection

This bounded Linux campaign checks that three functional assertions detect
specific defects. It compiles the original tests first, then builds each mutant
against an independent copy of the production headers. Your working headers
are never edited.

From the repository root on Linux or WSL with Clang and its sanitizer runtime:

```sh
python3 tests/mutation/run_detection.py --output out/mutations
```

All executables use Release flags (`-O3 -DNDEBUG`), symbols, strict warnings,
ASan and UBSan. Each selected original test must pass exactly once. Each mutant
must compile, exit with the runner's assertion-failure code and report the
planned assertion. A compiler error, timeout or sanitizer diagnostic fails the
campaign instead of counting as successful detection.

| Mutant | Regression | Expected assertion |
| --- | --- | --- |
| body-limit | Disable the configured Content-Length body budget | A body one byte above the limit is rejected |
| response-order | Send a completed successor before its pending predecessor | No output is sent while the predecessor is suspended |
| rejection-successor | Notify transport closure but omit the engine's terminal-state update | A later receive cannot dispatch a successor |

The detector tests live in
[regression_detection_tests.cpp](../unit/protocol/http/v11/regression_detection_tests.cpp)
and also run in the ordinary unit suite. They include accepted-boundary,
successful-order and terminal-close checks on the original code.

Each timestamped output directory preserves original header/test hashes, the
Git revision, compiler, compile commands in logs, each mutation diff, original
and mutant test logs, and `results.json`. The script verifies that original
headers and detector tests still have the same hashes before returning.
CI retains these diagnostics for 14 days and gates release on all three kills.
This is a demonstration for three selected regressions, not a mutation score
for the whole library.
