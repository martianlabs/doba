<a name="http-fuzzing"></a>
<h1>
  <a href="../../resources/tests/fuzz/h1-http-fuzzing.svg#gh-light-mode-only">
    <img src="../../resources/tests/fuzz/h1-http-fuzzing.svg" alt="HTTP fuzzing">
  </a>
  <a href="../../resources/tests/fuzz/h1-http-fuzzing-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/fuzz/h1-http-fuzzing-dark.svg" alt="HTTP fuzzing">
  </a>
</h1>

These tests give the HTTP decoder and chunked-body machinery malformed input
and check that valid messages still mean the same thing when delivered in
pieces. libFuzzer supplies the bytes; explicit property checks stay active
in optimized Release builds, including when assertions are disabled.

The canonical corpus contains **27 decoder seeds and 13 chunked seeds**:
valid pipelines, incomplete messages, conflicting framing, overflow,
extensions, trailers, query limits, and body/head limits. Generated valid
messages provide known answers alongside arbitrary input.

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
