<a name="test-security-and-integrity-audit"></a>
<h1>
  <a href="../resources/tests/security-audit/h1-test-security-and-integrity-audit.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h1-test-security-and-integrity-audit.svg" alt="Test security and integrity audit">
  </a>
  <a href="../resources/tests/security-audit/h1-test-security-and-integrity-audit-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h1-test-security-and-integrity-audit-dark.svg" alt="Test security and integrity audit">
  </a>
</h1>

Assessment date: 2026-11-04. Scope: the working tree, including the current
TCP, TLS and cooperative HTTP processing implementations on Linux and Windows.

The suite provides substantial contract and fault coverage. It does **not**
establish the absence of exploitable bugs, races or memory corruption. There
is no defensible percentage or "extremely high" security certification derived
from a green CTest run. This audit found reproducible weaknesses in test
oracles and child-process diagnostics and added independent checks for them.
No production header changes were needed for those corrections.

<a name="method-and-scope"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-method-and-scope.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-method-and-scope.svg" alt="Method and scope">
  </a>
  <a href="../resources/tests/security-audit/h2-method-and-scope-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-method-and-scope-dark.svg" alt="Method and scope">
  </a>
</h2>

The initial inventory contains 277 test, support and documentation files,
52,753 lines and 1,355 source-level `DOBA_TEST` registrations in 185 files.
These are source counts: platform conditionals, deliberate runner failures and
focused binaries prevent interpreting them as tests executed by every CI job.

Every inventoried file was included in the structural scan: registrations,
assertions, exception handling, sleeps, build selection and helpers. Detailed
semantic review focused on the attack boundaries below, shared test oracles,
runner failure propagation, ownership and native concurrency probes. This is
not a claim that each of the 1,355 cases received an independent line-by-line
security proof. The baseline inventory, case catalog, source hashes and logs
are recorded locally in `out/security-audit/2026-11-04`.

Normal test execution in **Release is intentional and correct**. Compilation
in Debug without normal-suite execution is not a finding. `DOBA_EXPECT` is
independent of `assert`/`NDEBUG`; fuzz properties abort explicitly. Existing
All current test tooling is C++; checks fail explicitly in Release. The
historical interpreter-based wrappers described below have been migrated,
including their positive and negative controls.

<a name="threat-and-coverage-matrix"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-threat-and-coverage-matrix.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-threat-and-coverage-matrix.svg" alt="Threat and coverage matrix">
  </a>
  <a href="../resources/tests/security-audit/h2-threat-and-coverage-matrix-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-threat-and-coverage-matrix-dark.svg" alt="Threat and coverage matrix">
  </a>
</h2>

| Boundary | Existing evidence and additions | Remaining assurance limit |
| --- | --- | --- |
| HTTP framing / request smuggling | Decoder and header-rule cases cover conflicting CL/TE, duplicate fields, malformed targets, control bytes, fragmentation and exact budgets. Native hostile-request cases append a sentinel and verify it is never dispatched. Decoder/chunked fuzzers compare segmentation and storage with known valid messages. The new engine model follows rejection through dispatch and output. | No systematic differential campaign against independent servers or complete request-smuggling grammar campaign. A policy difference alone would not prove a bug. |
| Response integrity / splitting | Serialization and header-injection cases, independent native response parser and actual wire tests. New negative controls reject invalid field bytes, status ranges and malformed trailers; the independent C++ TLS oracle rejects conflicting framing. | Helpers deliberately implement their fixture's subset, not all HTTP response semantics. The independent oracle accepts final Content-Length responses only. |
| Ownership / use after free | RX poisoning, owned requests inspected after decoder destruction, task ownership/cancellation and reader lifetime tests. New model checks suspended parameter/header views and retired coroutine/readers. New native binary pipelines use default admission limits. | Finite schedules and payload bounds; no exhaustive allocator-failure campaign or general lifetime proof. |
| TCP / IOCP / epoll races | Native publication, completion, cancellation, restart, partial sends, EAGAIN, reader bounds and kernel-call fault probes. Linux TSan and Windows ASan/UBSan jobs; controlled barriers and repeated concurrency groups. | Windows ASan is not a race detector. Repetition is sampling, not exhaustive schedule exploration. |
| TLS authenticity / progress | Verified TLS 1.2/1.3 peers, tiny BIOs, KeyUpdate, source budgets, disconnect/cancellation and mixed processing. Added corrupted real authenticated records: zero plaintext delivered, terminal scheduling stays empty. Added incomplete-handshake clients alongside a healthy client. | OpenSSL itself is not audited here. No stateful TLS/transport fuzzer or proof against connection-cap exhaustion. |
| Resource limits / denial of service | Policy boundary tests, queue/output/source limits, admission/recovery, slow readers, disk spill, cleanup and descriptor/handle checks. Model varies request windows and turn budgets. | Logical byte limits are not whole-process RSS limits. Application allocations and backing reader storage need their own bounds. Idle/handshake timeout remains outside the current contract. |
| Filesystem / cache | Path traversal, encoded paths, links, containment, metadata invalidation, compressed variants, ranges and conditional responses; resident cache bounds, nonwaiting loading and invalidation races. | No stateful filesystem/cache fuzz campaign or exhaustive rename/link/disk-failure interleavings. Deployment permissions and hostile mount semantics remain separate concerns. |
| Authentication / routing | Basic/Bearer malformed credentials, challenge construction, duplicate authorization, header injection, typed parameters, route precedence and handler signatures. | Credential validators and authorization policy belong to applications. These tests do not certify authentication deployments. |
| Common storage / utilities | Reader/writer limits, moves, binary data, storage spill and cleanup, containers, date ownership, signals and task ownership/cancellation. | Functional cases do not measure source/branch coverage; numeric/type extremes and allocation failures require further targeted measurement. |
| Runner / CI trust | Subprocess negative controls verify failures, exceptions, watchdogs, cleanup and filters. Inventory rejects missing mandatory groups. New diagnostic tests reject sanitizer reports even from exit-zero children. Packages and examples are executed separately. | Remote workflow execution must still confirm the new configuration. Eight selected mutations are not a library-wide mutation score. |

<a name="confirmed-weaknesses-and-corrections"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-confirmed-weaknesses-and-corrections.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-confirmed-weaknesses-and-corrections.svg" alt="Confirmed weaknesses and corrections">
  </a>
  <a href="../resources/tests/security-audit/h2-confirmed-weaknesses-and-corrections-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-confirmed-weaknesses-and-corrections-dark.svg" alt="Confirmed weaknesses and corrections">
  </a>
</h2>

### <a href="../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> 1. Response oracles could accept malformed output

The old Python TLS-processing parser accepted duplicate case-insensitive
Content-Length fields, CL with Transfer-Encoding, and status 099. These inputs
were replayed directly against the original helper before replacement. The old
C++ helper also failed the newly added field/status negative controls through
functional assertions, not a compilation error or unrelated process crash.

The independent native parser now validates token field names, control bytes,
status ranges, trailer syntax and bounded chunked wire/trailer collection.
The independent C++ Content-Length oracle has head/body bounds, strict framing, duplicate
singleton checks and bounded decimal parsing. Positive controls preserve
legal whitespace, opaque bytes, repeatable fields and pipeline boundaries.
These findings concern test reliability; they are not evidence that production
doba emitted the malformed responses.

### <a href="../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> 2. Child sanitizer findings could be discarded

Some example/probe wrappers discarded captured stderr when terminating the
server. A successful exchange or an exit code alone cannot qualify that child.
All affected native runners now use one diagnostic checker. Its metatests
launch exit-zero children emitting ASan, LSan, MSan, TSan and UBSan reports and
require rejection. Harmless sanitizer initialization text remains accepted.
Existing C++ runner process checks remain in place.

### <a href="../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> 3. Reader coverage needed a realistic admission window

A one-slot, one-byte regression fixture tests a deliberately constrained
contract; it does not replace queued-reader coverage. The new native case uses
default HTTP admission policies, three concurrent clients, ten requests per
client and one/four workers. Each response has a distinct deterministic binary
payload, including 512 KiB sources, sync/async handlers and HEAD requests.
It checks every byte, length, request identity, ordered close, forbidden
successor dispatch and release of all shared source owners after stop.

### <a href="../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> 4. A shutdown test inferred scheduling from sleeps

The Windows input-during-stop case assumed that 150/50 ms sleeps established
the stop phase and did not inspect its received 8 MiB payload. It now observes
listener refusal while the callback remains held, verifies stop is pending,
then releases the callback and validates all 8 MiB. Winsock refusal is allowed
its actual bounded completion time; a 100 ms probe is insufficient on this
machine, where closed loopback ports report refusal after about two seconds.

The assertion remains strict about preventing another engine invocation.
Receipt of socket bytes is never used as proof of IOCP send completion.

<a name="new-stateful-checks"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-new-stateful-checks.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-new-stateful-checks.svg" alt="New stateful checks">
  </a>
  <a href="../resources/tests/security-audit/h2-new-stateful-checks-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-new-stateful-checks-dark.svg" alt="New stateful checks">
  </a>
</h2>

`fuzz/engine_case.h` is shared by a deterministic Release unit case and
`fuzz_engine`. Its independent output model checks:

- Mixed sync/async responses in request order with windows of one to four.
- Fragmented, subsequently overwritten RX; suspended header/parameter views.
- Inline and retained reader bodies, delayed source consumption and no output
  crossing an active source.
- Ordered Connection-close and four malformed-request frontiers; successors
  must not dispatch or emit output.
- Bounded progress and release of every managed coroutine/source owner.

The deterministic suite runs 144 configurations. Fuzz input is a bounded
control grammar (512 bytes), not raw HTTP. Generated requests number one to
twelve; bodies contain up to 128 bytes and output is capped at 64 KiB. The
canonical engine corpus has twelve seeds. Decoder and chunked raw-input
campaigns remain separate and retain their original bounds and seeds.

CI decoder jobs additionally build, replay and fuzz this model for 60 seconds
on each OS, preserving established job names. Inventory requires the new
model, native binary-integrity and diagnostic groups. Critical repetition
includes the model; native sanitizer repetition includes binary integrity.

The TLS core now exercises all eight combinations of processing interests
for both TLS versions and BIO capacities 1/127/17408: 48 invocations, each
including an authenticated-record corruption check. These counts describe
this core executable, not other TLS executables or total CTest case counts.

A test must respect the contract it claims to check: repeated close requests
from defensive late protocol entry are not duplicate transport disconnection
notifications. TCP stops engine entry once closing begins. The new TLS test
checks no plaintext or scheduling resurrection; it does not invent an
exactly-once protocol-close callback requirement.

<a name="qualification-and-interpretation"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-qualification-and-interpretation.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-qualification-and-interpretation.svg" alt="Qualification and interpretation">
  </a>
  <a href="../resources/tests/security-audit/h2-qualification-and-interpretation-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-qualification-and-interpretation-dark.svg" alt="Qualification and interpretation">
  </a>
</h2>

Recorded validation results are maintained alongside source hashes in the
local audit evidence. The new groups run in ordinary Release suites and in
targeted sanitizer builds. Fuzz campaigns are short regression campaigns,
not evidence that their state spaces have been exhausted. Sanitizer-free
results mean no findings in the executions performed, not proof of safety.

| Local execution | Result |
| --- | --- |
| Linux GCC Release, complete registered suite | 20/20 CTest groups passed; inventory passed. |
| Windows MSVC Release, complete registered suite | 21/21 CTest groups passed; inventory passed. |
| Linux ASan / UBSan / TSan, affected engine/wire/TLS/example/driver groups | 9/9 groups passed in each build. |
| Windows ASan / UBSan, affected groups including ordinary native integration | 10/10 groups passed in each build. |
| Engine model repetition | 200 repetitions passed under Linux TSan and Windows ASan. |
| Native binary-integrity repetition | 20 repetitions passed under Linux TSan and Windows ASan. |
| Exact Windows shutdown/input regression | 20 repetitions passed under ASan, including all 8 MiB contents. |
| New engine fuzzer, ASan/UBSan | Linux: 193,254 inputs; Windows: 113,319 inputs. Each campaign ran for 61 elapsed seconds with no finding. Canonical replay also passed. |
| Missing-group controls | Omitting each of the three new groups from private CTest inventories failed with its intended diagnostic on both OSes. |
| Configuration/source controls | YAML parsed; job names, matrices, triggers, permissions, release prerequisites and existing execution conditions unchanged. Production headers unchanged by hash; existing root CMake changes preserved. |

The total group counts are not individual case counts. Local sanitizer
selection is the affected subset, not a new complete remote CI matrix run.
The standalone native negative controls remain active under optimization.

No Git operation, publication or remote CI run is part of this audit.

<a name="prioritized-remaining-work"></a>
<h2>
  <a href="../resources/tests/security-audit/h2-prioritized-remaining-work.svg#gh-light-mode-only">
    <img src="../resources/tests/security-audit/h2-prioritized-remaining-work.svg" alt="Prioritized remaining work">
  </a>
  <a href="../resources/tests/security-audit/h2-prioritized-remaining-work-dark.svg#gh-dark-mode-only">
    <img src="../resources/tests/security-audit/h2-prioritized-remaining-work-dark.svg" alt="Prioritized remaining work">
  </a>
</h2>

The [Release qualification follow-up](qualification/README.md) now measures
per-platform source coverage, gates module/header regressions and runs seeded
fault/ownership campaigns. Its baseline report distinguishes measured code from
unmapped headers. The earlier validation table above predates the additional
qualification and Winsock fault groups.

| Priority | Work | Acceptance evidence |
| --- | --- | --- |
| Measured locally; CI configured | Release line and condition-outcome coverage for parser, engine, native contexts and TLS adapter. | Per-platform LLVM exports, source-union reports, measured floors and explicit unmapped inventories; first remote run remains pending. |
| P1 | Expand stateful fuzzing to native transport/TLS lifecycle and filesystem/cache operations. | Persistent corpora, source revision/hashes, bounded operation models, fault schedules and minimal regression cases for every finding. |
| P1 | Sweep deterministic allocation, spill, short-read/write and kernel failures through lifecycle boundaries. | Exact failure diagnostics, no successor dispatch, owner/frame/handle cleanup and successful recovery where the contract permits it. |
| P1 | Add targeted synchronization mutations and controlled schedules. | Tests detect removed publication ordering, double completion, late timer and stale context events by the intended assertion, independently on both ports. |
| P2 | Isolate helper deadline arithmetic from scheduler-sensitive wire timing. | Fake-clock/readiness tests distinguish deadline reset errors; retain bounded real-wire smoke tests. Existing 600/850 ms helper timing checks remain sensitive to severe runner stalls. |
| P2 | Extend malformed grammar and numeric/policy extremes with generated valid controls. | Replayable cases around zero, limit, limit+1 and overflow, with bounded test allocations and independent expected results. |
| Configured; remote execution pending | Nightly twelve-round campaigns, 600-second fuzz budgets and persisted engine corpora. | Reproducible order/inputs and artifacts; the configured budget must not be presented as an exhaustive campaign. Longer soak testing remains separate. |

These are assurance gaps, not confirmed production vulnerabilities. The highest
priority is to measure what remains uncovered and force meaningful failure
schedules, rather than accumulate repeated green runs of unchanged cases.
