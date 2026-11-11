<a name="http-fuzzing"></a>
<h1>
  <a href="../../resources/tests/fuzz/h1-http-fuzzing.svg#gh-light-mode-only">
    <img src="../../resources/tests/fuzz/h1-http-fuzzing.svg" alt="HTTP fuzzing">
  </a>
  <a href="../../resources/tests/fuzz/h1-http-fuzzing-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/fuzz/h1-http-fuzzing-dark.svg" alt="HTTP fuzzing">
  </a>
</h1>

Give the parser strange bytes. Split the good messages into awkward pieces.
See whether the answers still make sense. That's the job here.

libFuzzer starts with 36 decoder seeds, 13 chunked-body seeds and 12 engine
seeds. CI replays them, then explores for 60 seconds per harness and OS,
or 600 seconds nightly, under ASan/UBSan. It keeps the growing corpora and
anything needed to reproduce a failure.

<a name="properties"></a>
<h2>
  <a href="../../resources/tests/fuzz/h2-properties.svg#gh-light-mode-only">
    <img src="../../resources/tests/fuzz/h2-properties.svg" alt="What we check">
  </a>
  <a href="../../resources/tests/fuzz/h2-properties-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/fuzz/h2-properties-dark.svg" alt="What we check">
  </a>
</h2>

| Harness | What must stay right |
| --- | --- |
| `fuzz_decoder` | Message boundaries, fields and body bytes match across whole/fragmented input and memory/disk storage. Owned requests survive overwritten buffers and decoder destruction. |
| `fuzz_chunked` | Payload, progress and message boundaries survive different fragments and output buffer sizes. Finishing or failing stays terminal. |
| `fuzz_engine` | Mixed handlers reply in order, rejected successors stay out, and coroutine/reader owners get released. |

Generated valid messages supply known answers too. Comparing two broken paths
wouldn't tell us much. Checks stay active in Release, including with `NDEBUG`.

Decoder/chunked input is capped at 16 KiB, accepted bodies at 4 KiB and generated
payloads at 512 bytes. Rejected-byte counts may differ with fragmentation;
we don't pretend they must match.

The engine takes a small control input: up to 512 bytes, twelve requests and
128-byte bodies. The same model runs 144 fixed configurations in the unit
suite. It doesn't fuzz raw HTTP, native sockets or TLS.

Build targets and flags live in [CMakeLists.txt](CMakeLists.txt).
[Still on the list](../security-audit.md#prioritized-remaining-work): broader
stateful fuzzing and longer stress runs.
