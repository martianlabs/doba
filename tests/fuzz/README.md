<a name="http-fuzzing"></a>
<h1>
  <a href="../../resources/tests/fuzz/h1-http-fuzzing.svg#gh-light-mode-only">
    <img src="../../resources/tests/fuzz/h1-http-fuzzing.svg" alt="HTTP fuzzing">
  </a>
  <a href="../../resources/tests/fuzz/h1-http-fuzzing-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/fuzz/h1-http-fuzzing-dark.svg" alt="HTTP fuzzing">
  </a>
</h1>

These Linux Clang harnesses run in optimized Release builds with libFuzzer,
ASan, UBSan and symbols. Instrumentation stays private to the fuzz targets.
`DOBA_BUILD_FUZZERS` defaults to `OFF`; Debug and unsupported compilers are
rejected when it is enabled.

<a name="build-and-run"></a>
<h2>
  <a href="../../resources/tests/fuzz/h2-build-and-run.svg#gh-light-mode-only">
    <img src="../../resources/tests/fuzz/h2-build-and-run.svg" alt="Build and run">
  </a>
  <a href="../../resources/tests/fuzz/h2-build-and-run-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/fuzz/h2-build-and-run-dark.svg" alt="Build and run">
  </a>
</h2>

From the repository root on Linux or WSL:

```sh
set -eu
cmake -S . -B out/build/fuzz -G Ninja \
  -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=clang++ \
  -DDOBA_BUILD_FUZZERS=ON -DDOBA_BUILD_TESTS=OFF \
  -DDOBA_BUILD_EXAMPLES=OFF -DDOBA_ENABLE_STRICT_WARNINGS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build out/build/fuzz
out/build/fuzz/tests/fuzz/doba_fuzz_seed_corpus out/fuzz-corpus
mkdir -p out/fuzz-artifacts/decoder out/fuzz-artifacts/chunked
export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1
export UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1
for harness in decoder chunked; do
  out/build/fuzz/tests/fuzz/fuzz_$harness \
    -runs=0 -timeout=5 -rss_limit_mb=2048 -max_len=16384 \
    -artifact_prefix=out/fuzz-artifacts/$harness/ out/fuzz-corpus/$harness
  out/build/fuzz/tests/fuzz/fuzz_$harness \
    -max_total_time=60 -timeout=5 -rss_limit_mb=2048 -max_len=16384 \
    -dict=tests/fuzz/http.dict -print_final_stats=1 \
    -artifact_prefix=out/fuzz-artifacts/$harness/ out/fuzz-corpus/$harness
done
```

Use a native Linux temporary directory for WSL runs. The C++ generator writes
27 decoder and 13 chunked canonical seeds as bytes, independent of Git's line
ending conversion. It preserves additional corpus entries. Cases mirror the
existing decoder, chunked framer/reader and integration security tests and
include valid pipelines, incomplete messages, conflicting framing, overflow,
extensions, trailers, query limits and body/head limits.

<a name="properties"></a>
<h2>
  <a href="../../resources/tests/fuzz/h2-properties.svg#gh-light-mode-only">
    <img src="../../resources/tests/fuzz/h2-properties.svg" alt="Properties">
  </a>
  <a href="../../resources/tests/fuzz/h2-properties-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/fuzz/h2-properties-dark.svg" alt="Properties">
  </a>
</h2>

`fuzz_decoder` checks input consumption, progress, accepted message boundaries,
fields, headers, query parameters and payload equivalence across whole and
fragmented delivery under identical policies. It compares memory storage with
immediate or threshold-based disk spill. Requests are made owned, caller buffers
are overwritten, and bodies and views are accessed after decoder destruction.
Every input also supplies binary payload bytes for a generated valid pipeline
with a known body, target paths and exact message boundaries.

Invalid streams stop at rejection. Fragmentation can change how many bytes were
scanned before an error, so rejected-byte counts are not compared. Interim
responses are excluded from semantic comparison. This harness exercises the
decoder; dispatch after rejection is an engine/integration-test responsibility.

`fuzz_chunked` checks consumption, terminal-state stability, fragmentation,
output-capacity equivalence and reader/framer message boundaries. Generated
valid bodies additionally check exact payload and preservation of a successor.
Reader and framer errors are not equated: only the framer has a payload budget.
Property checks use an explicit abort and remain active under `NDEBUG`.

Inputs are capped at 16 KiB, accepted request payloads at 4 KiB, and generated
known payloads at 512 bytes. Head, extension and trailer budgets vary with input
size; each equivalence comparison keeps the same policy. These bounds define
the short campaigns' scope rather than covering every production configuration.

<a name="ci-and-failure-reproduction"></a>
<h2>
  <a href="../../resources/tests/fuzz/h2-ci-and-failure-reproduction.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/fuzz/h2-ci-and-failure-reproduction-narrow.svg">
      <img src="../../resources/tests/fuzz/h2-ci-and-failure-reproduction.svg" alt="CI and failure reproduction">
    </picture>
  </a>
  <a href="../../resources/tests/fuzz/h2-ci-and-failure-reproduction-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 600px)" srcset="../../resources/tests/fuzz/h2-ci-and-failure-reproduction-narrow-dark.svg">
      <img src="../../resources/tests/fuzz/h2-ci-and-failure-reproduction-dark.svg" alt="CI and failure reproduction">
    </picture>
  </a>
</h2>

Each CI harness replays its restored corpus and then runs for 60 seconds, with
a five-second per-input timeout and a 2 GiB RSS limit. Corpus discoveries are
cached between successful runs. Fourteen-day artifacts retain corpus, crash or
timeout inputs, logs, revision, compiler, platform, CMake configuration, compile
commands and the executable. Failed campaigns fail CI and gate release.

Rebuild the recorded revision and run a saved input directly:

```sh
out/build/fuzz/tests/fuzz/fuzz_decoder out/fuzz-artifacts/decoder/crash-HASH
out/build/fuzz/tests/fuzz/fuzz_decoder -minimize_crash=1 -max_total_time=60 \
  -exact_artifact_path=out/fuzz-artifacts/decoder/minimized-HASH \
  out/fuzz-artifacts/decoder/crash-HASH
```

Use the matching executable for chunked inputs. Preserve the original artifact
and diagnostics before minimization. Classify a failure as a harness contract
mistake or a product defect; confirmed defects need deterministic regressions
and a separate fix. Do not suppress a property merely to make a campaign pass.
For candidate verification, use 600 seconds per harness on the exact candidate
revision, retaining its corpus and evidence. No nightly schedule is created.
