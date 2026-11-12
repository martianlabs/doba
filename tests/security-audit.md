<a name="test-security-and-integrity-audit"></a>
<h1>
  <a href="../resources/tests/security-audit/h1-test-security-and-integrity-audit.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h1-test-security-and-integrity-audit.svg" alt="Checking the tests">
  </a>
  <a href="../resources/tests/security-audit/h1-test-security-and-integrity-audit-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h1-test-security-and-integrity-audit-dark.svg" alt="Checking the tests">
  </a>
</h1>

Tests can be wrong too. This is what the **2026-11-04** review found, what we
fixed and what still needs work. It covers TCP, TLS and cooperative HTTP on
Linux and Windows.

<a name="method-and-scope"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-method-and-scope.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-method-and-scope.svg" alt="How we checked">
  </a>
  <a href="../resources/tests/security-audit/h2-method-and-scope-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-method-and-scope-dark.svg" alt="How we checked">
  </a>
</h2>

We looked for weak assertions, lost child-process errors, timing guesses and
ownership mistakes. The closer review focused on parsing, reply bytes,
lifetimes and native concurrency. It wasn't a line-by-line proof of every test.
The original inventory and logs are in `out/security-audit/2026-11-04`.

<a name="threat-and-coverage-matrix"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-threat-and-coverage-matrix.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-threat-and-coverage-matrix.svg" alt="What we cover">
  </a>
  <a href="../resources/tests/security-audit/h2-threat-and-coverage-matrix-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-threat-and-coverage-matrix-dark.svg" alt="What we cover">
  </a>
</h2>

| Area | What gets checked | Still to push harder |
| --- | --- | --- |
| HTTP input | Conflicting framing, bad bytes, fragments, exact limits and rejection before successor dispatch. | Broader malformed grammars and comparisons with independent servers. |
| Reply bytes | Serialization, header injection and independent parsers reading real socket output. | Helpers cover their test cases, not every HTTP response form. |
| Ownership | Overwritten RX, requests outliving decoders, cancelled tasks and retained readers. | More allocation failures and schedules. |
| Native TCP | Partial sends, EAGAIN, early IOCP completions, stale wakes, stop/restart and resource cleanup. | More forced races. Windows ASan isn't a race detector. |
| TLS | Verified peers, tiny BIOs, KeyUpdate, corrupt records and incomplete handshakes beside healthy clients. | Stateful native/TLS fuzzing and connection exhaustion. |
| Resource limits | Admission, queued bytes, slow readers, disk spill and recovery. | Long-running load. Logical byte limits aren't RSS limits. |
| Files, cache and routes | Traversal, links, invalidation races, route choice and credential parsing. | Wider filesystem fault sequences; application authorization remains the app's job. |
| Runners | Failed assertions, timeouts, missing groups and child sanitizer reports. | Keep checking that failures fail for the intended reason. |

<a name="confirmed-weaknesses-and-corrections"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-confirmed-weaknesses-and-corrections.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-confirmed-weaknesses-and-corrections.svg" alt="What needed fixing">
  </a>
  <a href="../resources/tests/security-audit/h2-confirmed-weaknesses-and-corrections-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-confirmed-weaknesses-and-corrections-dark.svg" alt="What needed fixing">
  </a>
</h2>

<a name="1-response-oracles-could-accept-malformed-output"></a>

### <a href="../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> The response checker was too forgiving

The old TLS helper accepted duplicate Content-Length, CL with Transfer-Encoding
and status 099. The C++ helper also missed invalid field/status cases.
Both now reject those inputs, with valid replies as positive controls.
This was a checker bug; it didn't show that doba sent those replies.

<a name="2-child-sanitizer-findings-could-be-discarded"></a>

### <a href="../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Good replies could hide bad shutdowns

Some wrappers threw away stderr when stopping a child server. They now inspect
it too. Tests feed them ASan/LSan/MSan/TSan/UBSan reports from children that exit
zero and require failure. Harmless sanitizer startup messages remain okay.

<a name="3-reader-coverage-needed-a-realistic-admission-window"></a>

### <a href="../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Readers needed a busier test

A one-slot fixture wasn't enough. The new socket case uses default HTTP limits,
three clients, ten requests each and one/four workers. It mixes sync/async
handlers, HEAD and binary readers up to 512 KiB, then checks every byte, reply
order, forbidden successors and released source owners.

<a name="4-a-shutdown-test-inferred-scheduling-from-sleeps"></a>

### <a href="../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Sleeping was not synchronization

A Windows stop test guessed its phase with sleeps and never inspected its
8 MiB reply. It now holds the callback, observes listener refusal, checks that
stop is waiting, releases the callback and validates the whole payload.
Receiving bytes still isn't proof of an IOCP completion.

<a name="new-stateful-checks"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-new-stateful-checks.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-new-stateful-checks.svg" alt="Following a whole conversation">
  </a>
  <a href="../resources/tests/security-audit/h2-new-stateful-checks-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-new-stateful-checks-dark.svg" alt="Following a whole conversation">
  </a>
</h2>

The [engine model](fuzz/engine_case.h) follows mixed handlers, fragmented and
overwritten input, delayed readers and rejection through to ordered output.
It checks progress and owner cleanup too. The fixed suite and fuzzer share
the model; [input bounds live here](fuzz/README.md).

TLS checks corrupt real authenticated records and require no delivered plaintext
or revived processing. Native clients also leave handshakes unfinished while
a healthy peer makes progress. That doesn't create a library handshake timeout.

<a name="qualification-and-interpretation"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-qualification-and-interpretation.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-qualification-and-interpretation.svg" alt="Runs and results">
  </a>
  <a href="../resources/tests/security-audit/h2-qualification-and-interpretation-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-qualification-and-interpretation-dark.svg" alt="Runs and results">
  </a>
</h2>

Local results from the 2026-11-04 review, before the later qualification groups:

| Run | Result |
| --- | --- |
| Full Release suites | Linux GCC 20/20 groups; Windows MSVC 21/21. Inventories passed. |
| Affected sanitizer groups | Linux ASan/UBSan/TSan: 9/9 each; Windows ASan/UBSan: 10/10 each. |
| Repeats | Engine model: 200; binary integrity: 20; Windows 8 MiB stop case: 20. All passed in their selected sanitizer builds. |
| Engine fuzzing, ASan/UBSan | 193,254 Linux and 113,319 Windows inputs, 61 seconds each; replay passed too. |
| Runner controls | Missing groups and exit-zero sanitizer reports failed as intended. |

The [coverage snapshot](qualification/BASELINE.md) and
[policy runs](qualification/POLICIES.md#local-validation-2026-11-11) record the
later checks. Passing these runs means no findings in those runs, not that
we've run out of bugs to find.

<a name="prioritized-remaining-work"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-prioritized-remaining-work.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-prioritized-remaining-work.svg" alt="Still on the list">
  </a>
  <a href="../resources/tests/security-audit/h2-prioritized-remaining-work-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-prioritized-remaining-work-dark.svg" alt="Still on the list">
  </a>
</h2>

- Fuzz native TCP/TLS lifecycles and filesystem/cache operations with sequences
  of actions, not just input bytes.
- Force allocation, spill, short-read/write and kernel failures at more points.
  Check cleanup and recovery each time.
- Remove selected synchronization steps in private copies and make controlled
  schedules catch double completion, late timers and stale contexts.
- Add more numeric/grammar extremes and make helper clock tests less sensitive
  to a busy runner. The 600/850 ms helper checks can still be upset by long stalls.
- Run sustained overload for longer and check recovery after the load drops.

The [release tools](qualification/README.md) already add measured coverage,
repeatable case order and longer nightly fuzz budgets. The next useful test
forces a missed failure path; another unchanged green run may not tell us much.
