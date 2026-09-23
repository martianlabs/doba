<a name="backlog"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h1-backlog-dark.svg">
    <img src="../resources/docs/backlog/h1-backlog.svg" alt="Backlog">
  </picture>
</h1>

[Index](HANDOFF.md)

Source of truth for doba's outstanding implementation, verification, and
contract decisions. Identifiers remain stable when entries are removed;
numbering does not express priority or implementation order.
Engineering and verification requirements are defined in
[QUALITY.md](QUALITY.md).

An outstanding item does not imply that its design has been decided.
Open decisions and deferred scope are stated explicitly. Priorities without
a selected value are marked "Not set"; complexity estimates are separate.

<a name="contents"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-contents-dark.svg">
    <img src="../resources/docs/backlog/h2-contents.svg" alt="Contents">
  </picture>
</h2>

- [Release target](#release-target)
- [Inventory](#inventory)
- [Operational hardening](#operational-hardening)
- [Bugs](#bugs)
- [Product and convenience](#product-and-convenience)
- [Quality and validation](#quality-and-validation)
- [Release engineering](#release-engineering)
- [C++ maintainability](#c-maintainability)
- [Public documentation](#public-documentation)
- [Beyond the first release](#beyond-the-first-release)

<a name="release-target"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-release-target-dark.svg">
    <img src="../resources/docs/backlog/h2-release-target.svg" alt="Release target">
  </picture>
</h2>

The frozen target is `0.1.0-beta1`, a beta for adoption and evaluation in
controlled deployments. This section is the authoritative release scope.
B14-B19 migration bugs are resolved; their records remain below as evidence.
Other outstanding work outside the release scope remains future work and
does not block this beta.

Implementation order:

1. B14-B19: completed; all migration regression tests pass.
2. B9: completed; response-driven closure verified on both platforms.
3. F1 and F2: implement TLS and GZIP response compression.
4. C1, C2, C3 and C7: implement and verify all operational limits.
5. RE1: complete minimum documentation, the full existing CI and CMake consumer
   validations, and coherent versioning and publication.

**Required capabilities.** F1 (TLS) and F2 (GZIP) are mandatory for the
selected HttpArena participation scope. They take priority over C1/C2/C3/C7
and RE1. Dependency choices and public APIs require separate implementation
plans; this scope decision does not select them.

**Operational policies.** C1, C2, C3 and C7 are mandatory for this release.
Policies will be injected when the corresponding modules are created.
A later design stage, before implementation and publication, will define
policy contracts, module responsibilities, injection APIs, ownership,
defaults, and boundary behavior. No concrete policy types or signatures are
selected here. HTTP request limits belong to the protocol layer; connection,
inactivity and pending-work limits belong to their responsible modules.

**Exit criteria.**

- B9 verified: synchronous, immediate and deferred response-driven closure
  passes on Windows and Linux, including complete bodies before EOF and the
  pipelined close boundary. No production change was needed.
- Implement F1 and F2 with focused unit and real-socket tests on Windows
  and Linux. Verify TLS handshake, encrypted HTTP delivery, closure and
  errors; verify GZIP negotiation, response framing and decoded payloads
  for empty, binary and large bodies.
- Implement C1, C2, C3 and C7 with focused boundary and real-socket tests on
  IOCP and epoll. Include slow clients, resource release, ordering and
  cancellation where relevant. Assessing or documenting a limit is not a
  substitute for implementing it.
- Pass every existing gate in [QUALITY.md](QUALITY.md) on the exact release
  revision: compiler/configuration matrix, strict warnings, sanitizers,
  minimum CMake, and isolated package and source consumers.
- Publish minimum usage preconditions and deployment limitations, release
  notes, a working vulnerability-reporting channel, and a version and tag
  that consistently identify `0.1.0-beta1`.

**Future work.** B13, P2-P8, QA1-QA3, QA5-QA6, DT1-DT3, DOC1-DOC2 and F3-F7
remain outside this release. RE1 also retains the full contribution guide
as future work. Minimum usage documentation is part of RE1; completing the
broader documentation or API work in DT2/DT3/DOC1/DOC2 is not required.
Focused tests for release items remain mandatory without requiring the
exhaustive coverage, fuzzing, benchmarks, soak campaigns or external-tool
automation described by the future QA entries.

**Scope control.** Until publication, work is limited to the listed release
items, their tests, the minimum release closure, and B14-B19 migration fixes.
Optional functionality, refactoring and broader quality programs remain
future work. Any addition
requires an explicit release-scope decision, supported by concrete evidence
when a newly reproduced defect affects the supported behavior.

<a name="inventory"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-inventory-dark.svg">
    <img src="../resources/docs/backlog/h2-inventory.svg" alt="Inventory">
  </picture>
</h2>

30 outstanding entries across eight categories: seven entries with release
scope and twenty-three future entries. No critical migration bug remains.
B14-B19 originally recorded 31 failing tests on 2026-09-21. After the
2026-09-23 B19 fix, all tests pass on Windows and WSL.
Resolved B9 and B14-B19 are retained below as evidence and excluded from
the outstanding totals.
Verification pending means that the focused regressions or runtime
measurements described by an entry remain to be run. Category totals count
entries, not individual failing tests; RE1 separates its minimum release
work from the future contribution guide.

| Category | Identifiers | Migration | Release | Future work | Total |
| --- | --- | --- | --- | --- | --- |
| Operational hardening | C1-C3, C7 | 0 | 4 | 0 | 4 |
| Bugs | B13 | 0 | 0 | 1 | 1 |
| Product and convenience | F1-F2, P2-P8 | 0 | 2 | 7 | 9 |
| Quality and validation | QA1-QA3, QA5-QA6 | 0 | 0 | 5 | 5 |
| Release engineering | RE1 | 0 | 1 | 0 | 1 |
| C++ maintainability | DT1-DT3 | 0 | 0 | 3 | 3 |
| Public documentation | DOC1-DOC2 | 0 | 0 | 2 | 2 |
| Beyond the first release | F3-F7 | 0 | 0 | 5 | 5 |
| **Total** | | **0** | **7** | **23** | **30** |

| Item | Category | Status | Priority | Target |
| --- | --- | --- | --- | --- |
| [B14](#b14-asynchronous-handler-execution) | Critical bug | Resolved; verified 2026-09-22 | P0 (maximum) | Current migration |
| [B15](#b15-missing-100-continue) | Critical bug | Resolved; verified 2026-09-22 | P0 (maximum) | Current migration |
| [B16](#b16-missing-http-rejection-responses) | Critical bug | Resolved; verified 2026-09-23 | P0 (maximum) | Current migration |
| [B17](#b17-http-error-response-content) | Critical bug | Resolved; verified 2026-09-22 | P0 (maximum) | Current migration |
| [B18](#b18-empty-delivery-contract) | Critical bug | Resolved; verified 2026-09-23 | P0 (maximum) | Current migration |
| [B19](#b19-send-limit-failure-notification) | Critical bug | Resolved; verified 2026-09-23 | P0 (maximum) | Current migration |
| [C1](#c1-single-inactivity-timeout) | Hardening | Pending | Release gate | 0.1.0-beta1 |
| [C2](#c2-effective-per-request-limits) | Hardening | Pending | Release gate | 0.1.0-beta1 |
| [C3](#c3-global-active-connection-limit) | Hardening | Pending | Release gate | 0.1.0-beta1 |
| [C7](#c7-pending-response-and-work-budget) | Hardening | Design and validation pending | Release gate | 0.1.0-beta1 |
| [B9](#b9-response-driven-connection-close) | Bug | Resolved; verified 2026-09-23 | Release gate | 0.1.0-beta1 |
| [B13](#b13-signed-overflow-in-the-httparena-adapter) | Benchmark bug | Verification pending | Medium | Future work |
| [F1](#f1-tls) | Product | Design and implementation pending | Release gate | 0.1.0-beta1 |
| [F2](#f2-compression-and-gzip) | Product | Design and implementation pending | Release gate | 0.1.0-beta1 |
| [P2](#p2-access-logging) | Product | Pending | Not set | Future work |
| [P3](#p3-middleware-chain) | Product | Pending | Not set | Future work |
| [P4](#p4-form-parsing) | Product | Pending | Not set | Future work |
| [P5](#p5-automatic-conditionals-and-ranges) | Product | Deferred | Not set | Future work |
| [P6](#p6-output-trailers) | Product | Deferred | Not set | Future work |
| [P7](#p7-automatic-resource-options) | Product | Deferred | Not set | Future work |
| [P8](#p8-429-response-construction) | Product | Pending | Low | Future work |
| [QA1](#qa1-exhaustive-compliance-suite) | QA | Pending | Not set | Future work |
| [QA2](#qa2-fuzzing) | QA | Deferred | High | Future work |
| [QA3](#qa3-performance-baseline) | QA | Pending | Medium | Future work |
| [QA5](#qa5-stress-campaigns) | QA | Pending | Not set | Future work |
| [QA6](#qa6-external-compliance-automation) | QA | Deferred | Not set | Future work |
| [RE1](#re1-release-governance-and-traceability) | Release | Pending | Release gate | 0.1.0-beta1 |
| [DT1](#dt1-platformh-dependencies-and-global-effects) | C++ | Pending | Medium | Future work |
| [DT2](#dt2-indexed-getter-contract) | C++ | Pending | Low/Medium | Future work |
| [DT3](#dt3-response-framing-and-capacity-contract) | C++ | Decision pending | Medium | Future work |
| [DOC1](#doc1-transport-lifecycle) | Documentation | Pending | Not set | Future work |
| [DOC2](#doc2-request-views-and-getters) | Documentation | Pending | Not set | Future work |
| [F3](#f3-progressive-streaming-and-sse) | Future | Deferred | Not set | Future work |
| [F4](#f4-ordered-upgrade-barrier) | Future | Deferred | Not set | Future work |
| [F5](#f5-websockets) | Future | Deferred | Not set | Future work |
| [F6](#f6-listener-and-worker-configuration) | Future | Deferred | Not set | Future work |
| [F7](#f7-shutdown-with-draining) | Future | Deferred | Not set | Future work |

<a name="operational-hardening"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-operational-hardening-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-operational-hardening-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-operational-hardening-dark.svg">
    <img src="../resources/docs/backlog/h2-operational-hardening.svg" alt="Operational hardening">
  </picture>
</h2>

C1, C2, C3 and C7 are all required for `0.1.0-beta1`. Their policies will
be defined in the later design stage described in the release target.

<a name="c1-single-inactivity-timeout"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c1-single-inactivity-timeout-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c1-single-inactivity-timeout-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c1-single-inactivity-timeout-dark.svg">
    <img src="../resources/docs/backlog/h3-c1-single-inactivity-timeout.svg" alt="C1: Single inactivity timeout">
  </picture>
</h3>

**Context.** Neither backend currently closes an open connection that stops
making progress. Original complexity estimate: M.

**Scope.** A single `inactivity_timeout`, supplied through a policy injected
when the responsible module is created, applies to reads, keep-alive, and
writes. Receiving or sending bytes renews the deadline; the total duration
of a connection or request does not exhaust it while progress continues.
On expiry, the transport closes safely.

**Components.** Transport configuration, `tcpip_windows.h`,
`tcpip_linux.h`, and TCP/IP integration tests.

**Acceptance and tests.**

- A fragmented read that makes progress keeps the connection alive.
- A stalled partial request, idle keep-alive, and a client that does not read
  cause closure when the applicable deadline expires.
- Check timing, lifetime, ordering, and exactly one callback per closure.
- Run equivalent scenarios in IOCP and epoll.

**Dependencies and decisions.** Follow the policy design stage shared by
C1, C2, C3 and C7. Determine the injection contract, default value, possible
disabling behavior, and test timing tolerance before implementation.

**Delivery.** Required for `0.1.0-beta1`.

**Out of scope.** Separate deadlines per phase, an absolute maximum duration,
dynamic configuration, and automatic `408` responses.

<a name="c2-effective-per-request-limits"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c2-effective-per-request-limits-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c2-effective-per-request-limits-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c2-effective-per-request-limits-dark.svg">
    <img src="../resources/docs/backlog/h3-c2-effective-per-request-limits.svg" alt="C2: Effective per-request limits">
  </picture>
</h3>

**Remaining work.** Define request policies injected at module creation and
how the standard server supplies them to its decoder. Select defaults and
enforce the selected budget for chunked bodies, whose final size is not
known from the headers.

**Evidence.** [policies.h](../include/protocol/http/v11/policies.h) defaults
its five size/count limits to zero (unlimited), and the standard server has
no policy configuration API. The
[Content-Length interpreter](../include/protocol/http/v11/headers/content_length.h)
already rejects a value above a supplied policy limit. Reuse the existing
field checks; the missing work is configuration, effective defaults, and
cumulative body accounting. The internal head buffer does not bound body
storage, which can spill to disk.

**Components.** Policies, limits, context, decoder, server composition, and
their tests.

**Acceptance and tests.**

- Define what each limit counts, including encoded and decoded body bytes.
- Verify zero, limit-1, limit, and limit+1 through configured server requests.
- Check rejection before consuming an excessive Content-Length body and
  the corresponding rejection reason; cover chunked bodies separately.
- Verify that configuration cannot change unsafely during use.
- Measure whether the change affects the hot path.

**Dependencies and decisions.** Injection API shape, defaults, and body
accounting will be defined in the C1/C2/C3/C7 policy design stage. C2 controls
resources per request; C3 separately limits active connections.

**Delivery.** Required for `0.1.0-beta1`, including cumulative chunked-body
limits and their focused boundary tests.

<a name="c3-global-active-connection-limit"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c3-global-active-connection-limit-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c3-global-active-connection-limit-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c3-global-active-connection-limit-dark.svg">
    <img src="../resources/docs/backlog/h3-c3-global-active-connection-limit.svg" alt="C3: Global active connection limit">
  </picture>
</h3>

**Context.** `connections_` is observational and does not limit admission.
Original complexity estimate: M.

**Scope.** A global maximum supplied through a policy injected when the
responsible module is created. Each backend atomically reserves capacity
before admitting a context. Once the limit is reached, it immediately closes
the new connection while preserving those already admitted.

**Components.** Generic configuration, admission and closure in both backends,
the connection counter, and TCP/IP tests.

**Acceptance and tests.**

- Exceed the maximum with concurrent clients without exceeding capacity.
- Previously admitted connections remain served.
- Every closure releases exactly one reservation, including error paths.
- A new connection can enter once capacity is available again.
- Verify equivalent behavior on Windows and Linux.

**Dependencies and decisions.** Follow the C1/C2/C3/C7 policy design stage;
determine defaults and the semantics of any unlimited value. C3 controls
connections; C2 controls resources per request and C7 controls pending work.

**Delivery.** Required for `0.1.0-beta1`.

**Out of scope.** Per-worker quotas, dynamic changes, acceptance backpressure,
new callbacks, and HTTP rejection responses.

<a name="c7-pending-response-and-work-budget"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-dark.svg">
    <img src="../resources/docs/backlog/h3-c7-pending-response-and-work-budget.svg" alt="C7: Pending response and work budget">
  </picture>
</h3>

**Status and evidence.** Implementation and validation are required for
`0.1.0-beta1`. Source inspection found no per-connection budget at response
insertion or deferred slot reservation.
A socket send buffer limit does not bound queued response bodies or work.
Resource exhaustion has not been reproduced in a stress campaign.

**Risk.** A pipelining client that does not consume responses, or an early
deferred handler that delays ordered output, may accumulate responses and
retained request state while more requests are accepted.

**Components.** `responses_`, deferred reservations and receive scheduling in
[tcpip_linux.h](../include/transport/server/tcpip_linux.h) and
[tcpip_windows.h](../include/transport/server/tcpip_windows.h).

**Scope.** Implement limits on retained work and queued response bytes
through policies injected at module creation. Define accounting, defaults,
and admission/resume or rejection behavior in the C1/C2/C3/C7 policy design
stage. Preserve ordering, cancellation and ownership across synchronous and
deferred handlers without prescribing a new queue abstraction or an
HTTP-specific transport.

**Acceptance and tests.** Use a non-reading peer and a delayed first handler
with pipelined successors on both platforms. Check the configured work and
byte boundaries and the selected admission behavior. Measure pending entries
and retained memory, verify another connection remains serviceable, and check
resume, disconnect and stop without leaks or duplicate completion.

**Delivery.** Required for `0.1.0-beta1`. Deployment restrictions or proxy
mitigations do not replace this implementation. C3 bounds connections, not
this queue; F3 concerns future progressive output. Focused validation belongs
to C7 and does not depend on completing the future QA1/QA5 campaigns.

<a name="bugs"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-bugs-dark.svg">
    <img src="../resources/docs/backlog/h2-bugs.svg" alt="Bugs">
  </picture>
</h2>

B14-B19 were Critical severity and P0 (maximum) migration work; all are now
resolved. In the original 2026-09-21 run,
Windows unit 843/848 and integration 86/112 passed; WSL unit 841/846 and integration
86/112 passed. The same five unit and twenty-six integration cases failed on
both platforms. Builds used Debug and strict warnings, without ASan/UBSan.
These are recorded results, not a new test execution during this backlog edit.

The six entries assign every failing test once. Cross-references capture
shared dependencies; a failure before later assertions does not prove those
later behaviors are broken. B18 and B19 now have agreed, verified contracts.
All original migration-failure tests remain active with their assertions
unchanged. The separate unit test that reused a connection after a handler
error was aligned with B17: it now verifies closure and a new connection.
The latest 2026-09-23 full suites pass Windows unit 886/886, WSL unit
884/884 and integration 125/125 on both platforms. No migration failure
remains. B19 added three regressions and aligned source-limit notifications
with cancellation plus rejection. B9 verification adds six unit and three
integration cases, plus stronger lifecycle checks. Test-helper suites also
pass (16 unit and 11 integration checks on each platform).

B9 is separately verified and resolved below. B13 remains future benchmark
work and is not revalidated by this migration-failure inventory.

<a name="b14-asynchronous-handler-execution"></a>
<h3>B14: Asynchronous handler execution and lifecycle</h3>

**Status.** Resolved; verified 2026-09-22. **Severity.** Critical.
**Priority.** P0 (maximum). **Target.** Current migration.

**Original cause.** The HTTP
[engine](../include/protocol/http/v11/engine.h) returned 501 for asynchronous
routes and parametrized handlers without invoking them. That path has been
replaced with immediate/deferred execution and a per-connection response FIFO.

**Implemented behavior.** Asynchronous routes and controllers execute with
immediate completion or suspension/resumption. The engine preserves request
order, retains suspended request storage, and propagates cooperative
cancellation on shutdown, client reset and input EOF. Its pending-request
limit is configured through engine policies, independently of transport bytes.

**Acceptance and tests.** On 2026-09-22, all 17 cases below passed on
Windows and WSL, including deferred close draining, ordered errors, HEAD
error framing and interim responses behind suspended handlers.
Response ordering belongs to the engine; byte delivery and draining remain
transport responsibilities.

**Dependencies and limits.** The interim-ordering dependency on
[B15](#b15-missing-100-continue) and the deferred error/HEAD dependencies on
[B17](#b17-http-error-response-content) are now verified. Deferred close
coverage initially exercised request-driven closure;
[B9](#b9-response-driven-connection-close) now verifies response-driven closure.
Cancellation remains cooperative; a suspended awaitable
must eventually complete or respond to cancellation to release its frame.

| Suite | Test | Source | Windows / WSL |
| --- | --- | --- | --- |
| unit | server completes suspended async responses | [tests/unit/protocol/http/v11/server_tests.cpp](../tests/unit/protocol/http/v11/server_tests.cpp) | Passed |
| unit | server propagates async handler exceptions | [tests/unit/protocol/http/v11/server_tests.cpp](../tests/unit/protocol/http/v11/server_tests.cpp) | Passed |
| unit | server invokes parametrized async handlers | [tests/unit/protocol/http/v11/server_tests.cpp](../tests/unit/protocol/http/v11/server_tests.cpp) | Passed |
| unit | async handler observes cancellation after suspension | [tests/unit/protocol/http/v11/server_tests.cpp](../tests/unit/protocol/http/v11/server_tests.cpp) | Passed |
| integration | HTTP controllers preserve asynchronous response ordering | [tests/integration/protocol/http/v11/server_controller_tests.cpp](../tests/integration/protocol/http/v11/server_controller_tests.cpp) | Passed |
| integration | HTTP suspended controllers receive shutdown cancellation | [tests/integration/protocol/http/v11/server_controller_tests.cpp](../tests/integration/protocol/http/v11/server_controller_tests.cpp) | Passed |
| integration | HTTP/1.1 hides deferred handler and serializer diagnostics | [tests/integration/protocol/http/v11/server_response_tests.cpp](../tests/integration/protocol/http/v11/server_response_tests.cpp) | Passed |
| integration | HTTP/1.1 suppresses deferred HEAD error bodies in pipelines | [tests/integration/protocol/http/v11/server_response_tests.cpp](../tests/integration/protocol/http/v11/server_response_tests.cpp) | Passed |
| integration | HTTP/1.1 invokes an asynchronous parametrized route over TCP | [tests/integration/protocol/http/v11/server_routing_tests.cpp](../tests/integration/protocol/http/v11/server_routing_tests.cpp) | Passed |
| integration | HTTP/1.1 keeps synchronous responses behind a suspended handler | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |
| integration | HTTP/1.1 orders asynchronous responses by request order | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |
| integration | HTTP/1.1 retains suspended request views across later heads | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |
| integration | HTTP/1.1 cancels a suspended request after client reset | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |
| integration | HTTP/1.1 cancels a suspended request after input eof | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |
| integration | HTTP/1.1 drains a deferred close response before eof | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |
| integration | HTTP/1.1 keeps interim output behind an earlier deferred response | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |
| integration | HTTP/1.1 orders a deferred error before accepted successors | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |

<a name="b15-missing-100-continue"></a>
<h3>B15: Missing 100 Continue before request bodies</h3>

**Status.** Resolved; verified 2026-09-22. **Severity.** Critical.
**Priority.** P0 (maximum). **Target.** Current migration.

**Original cause.** The decoder recognized `Expect: 100-continue`, but
cleared its parsing context without reporting the accepted expectation to
the engine. Clients waiting for an interim response could not send the body.

**Implemented behavior.** `deserialization_result<RQty, RSty>` now carries
an optional response. After validating the complete head, the HTTP decoder
returns `kMoreBytesNeeded` with `RSty::continue_100()` once if body decoding
is incomplete. The engine emits it after all earlier responses have been
handed to the transport. It retains at most one interim response, without a
second FIFO or synchronization. Completion of the body discards an unsent
interim; errors, closing and shutdown prevent later emission.

RFC 9110 S10.1.1 permits omitting 100 when content has already arrived or
framing indicates no content. RFC 9112 S9.2 requires interim/final output to
remain associated with requests in arrival order. The existing serializer
omits body bytes, Content-Length and Transfer-Encoding for 100 responses.
Transports and generic engine/transport concepts are unchanged.

**Acceptance and tests.** All three cases below and B14's deferred-interim
ordering regression pass on Windows and WSL. Added unit coverage verifies
optional-response ownership, raw/chunked fragmented bodies, one provisional
per request, invalid/incomplete heads, completed/empty bodies, deferred
ordering, stop/error/close and delivery failure. The missing-interim engine
regressions failed before the fix. All 32 engine, 160 decoder and three result
unit tests pass on both platforms, as do all 17 B14 acceptance cases. Full
builds include examples; full suites retain nine unrelated known failures.

**Performance verification.** The same WSL mixed-request benchmark used
8 workers, 4096 connections, pipeline 1 and 133-byte responses with runtime
Date. Three-round medians were 573,650 RPS before B15, 536,780 with optional
and 430,530 for Actix. Additional paired runs also measured lower throughput
than the pre-B15 reference. Results varied substantially between runs; zero
regression is not established. An external unique_ptr result-field variant
passed 32 engine tests but did not outperform optional in either paired
comparison (-0.13% and -4.17%), so repository storage remains optional.
ASan/UBSan with leak detection passed all 195 focused tests.

**Limits.** Rejected request error responses remain tracked by B16. No
transport batching or additional queue limits are introduced here. Functional
closure does not establish a zero-cost performance result.

| Suite | Test | Source | Windows / WSL |
| --- | --- | --- | --- |
| integration | expect continue example sends interim before reading the body | [tests/integration/examples/http/v11/expect_continue_tests.cpp](../tests/integration/examples/http/v11/expect_continue_tests.cpp) | Passed |
| integration | HTTP/1.1 echoes a body after 100 Continue | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |
| integration | HTTP/1.1 sends one interim while a fragmented body is pending | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |

<a name="b16-missing-http-rejection-responses"></a>
<h3>B16: Missing HTTP responses for rejected requests</h3>

**Status.** Resolved; verified 2026-09-23. **Severity.** Critical.
**Priority.** P0 (maximum). **Target.** Current migration.

**Confirmed cause.** The engine closed invalid input without serializing a
rejection. A second failure appeared at receive capacity: closing TCP with
unread input reset the connection after sending the rejection. A Linux
syscall trace reproduced the 400 response followed by ECONNRESET.

**Implemented behavior.** The decoder supplies 400/413/414/417/431/501/505
responses through the existing optional deserialization response. Known
HEAD requests retain Content-Length but omit content, including failures
in a fragmented body (RFC 9110 S9.3.2 and S8.6). The engine preserves FIFO
order behind admitted requests, discards a superseded interim response,
and stops dispatch at the rejection boundary. Interim and rejection
responses share serialization; handler and router responses retain their
existing path. No new response wrapper or queue was added.

Both TCP transports drain output, shut down the write side, then discard
input until peer EOF (RFC 9112 S9.6). Windows keeps the receive buffer
reserved while the engine callback is using it. No thread, lock or timer
was added. Transport stop still drains output and releases connections
without waiting for peer EOF.

**Acceptance and tests.** All seven original cases below pass on Windows
and WSL with their original assertions. New tests cover rejection status
mapping, HEAD recognition and body fragments, ordering behind deferred and
unsafe handlers, superseded interim responses, failed delivery, and TCP
input disposal after output draining. The new transport test failed before
the transport fix and passes after it. Full-suite results are recorded above.
Clang AddressSanitizer and UndefinedBehaviorSanitizer pass 202 unit and 23
integration cases on Linux, with leak detection enabled.

**Performance.** Three rotated WSL rounds use the existing HttpArena
comparison: 8 workers, 4096 connections, pipeline 1, separate server/client
CPU affinity, 5-second warmup and 15-second samples. Both Doba revisions use
identical adapters/compiler flags and real 133-byte responses with runtime
Date. Median RPS: pre-B16 516,390, current 503,810,
Actix 418,670. Current vs pre-B16: -2.44%; vs Actix:
+20.34%. Every measured response is 2xx. Three WSL rounds do not
establish zero overhead or isolate the cause of the observed difference.

**Limits.** Normal graceful closure retains connection resources until peer
EOF; stop releases them after draining output. No timeout was introduced.
Aborted connections and transport stop do not promise graceful delivery
when the peer continues sending. B18 and B19 are resolved below.

| Suite | Test | Source | Windows / WSL |
| --- | --- | --- | --- |
| unit | server suppresses error bodies only for known HEAD requests | [tests/unit/protocol/http/v11/server_tests.cpp](../tests/unit/protocol/http/v11/server_tests.cpp) | Passed |
| integration | HTTP/1.1 terminates complete heads around decoder capacity | [tests/integration/protocol/http/v11/server_limits_tests.cpp](../tests/integration/protocol/http/v11/server_limits_tests.cpp) | Passed |
| integration | HTTP/1.1 preserves every query parameter at supported boundaries | [tests/integration/protocol/http/v11/server_limits_tests.cpp](../tests/integration/protocol/http/v11/server_limits_tests.cpp) | Passed |
| integration | HTTP/1.1 enforces chunk extension and trailer wire limits | [tests/integration/protocol/http/v11/server_limits_tests.cpp](../tests/integration/protocol/http/v11/server_limits_tests.cpp) | Passed |
| integration | HTTP/1.1 rejects hostile requests without dispatching successors | [tests/integration/protocol/http/v11/server_security_tests.cpp](../tests/integration/protocol/http/v11/server_security_tests.cpp) | Passed |
| integration | HTTP/1.1 rejects invalid header syntax and its successor | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |
| integration | HTTP/1.1 rejects invalid dispatched header values and its successor | [tests/integration/protocol/http/v11/server_tests.cpp](../tests/integration/protocol/http/v11/server_tests.cpp) | Passed |

<a name="b17-http-error-response-content"></a>
<h3>B17: HTTP error response content and HEAD framing</h3>

**Status.** Resolved; verified 2026-09-22. **Severity.** Critical.
**Priority.** P0 (maximum). **Target.** Current migration.

**Confirmed cause.** The engine built empty 500 responses for handler
exceptions and closed without an HTTP error when response serialization
failed. Neither path satisfied the established public error representation.

**Resolution.** Synchronous and deferred failures now produce a fresh 500
with the body "Internal Server Error". HEAD retains Content-Length 21 without
body bytes (RFC 9110 S9.3.2). A serialization failure before transport delivery
gets one replacement attempt; transport delivery failures are never retried
as another response. Failure to construct or serialize the replacement closes
the connection.

The engine stops accepting requests when it handles the error, preserves the
turns already admitted, and announces closure only on the last response.
The transport retains responsibility for draining bytes. No transport or
public API changes were needed.

**Verification.** Both cases below and the two deferred error/HEAD cases in
[B14](#b14-asynchronous-handler-execution) pass on Windows and WSL, including
all standard-exception, unknown-exception and invalid-framing scenarios,
closure and new-client recovery. Four new engine unit tests cover immediate
sync/async errors, deferred errors with unfinished successors, the admission
boundary, HEAD framing, and no duplicate delivery after transport failure.
The four regressions failed before the fix and pass afterwards. Full builds
including examples and full CTest runs completed on both platforms; the
13 failures remaining at B17 completion were pre-existing and outside B17.
B15 subsequently resolved four of them.

**Limits.** The exact error text and closing policy are project contracts,
not HTTP requirements for every 500. Recovery applies before bytes are handed
to the transport; a later body-reader or socket failure cannot be replaced
with a second HTTP response. The B14 dependency on B15 is now verified.

| Suite | Test | Source | Windows / WSL |
| --- | --- | --- | --- |
| integration | HTTP/1.1 converts response failures and recovers on new clients | [tests/integration/protocol/http/v11/server_response_tests.cpp](../tests/integration/protocol/http/v11/server_response_tests.cpp) | Passed |
| integration | HTTP/1.1 suppresses synchronous HEAD error bodies | [tests/integration/protocol/http/v11/server_response_tests.cpp](../tests/integration/protocol/http/v11/server_response_tests.cpp) | Passed |

<a name="b18-empty-delivery-contract"></a>
<h3>B18: Empty delivery closes the send queue</h3>

**Status.** Resolved; verified 2026-09-23. **Severity.** Critical.
**Priority.** P0 (maximum). **Target.** Current migration.

**Confirmed cause.** Both transports rejected zero bytes without a source.
They also used nonzero reserved bytes to identify an active delivery, so
removing the rejection alone could overwrite or lose an empty delivery.

**Agreed contract and implementation.** A zero-length buffer without a
source is accepted with either a null or nonnull buffer. It reserves no
send bytes and completes once in FIFO order, without reentering the engine
callback. Cancellation before its turn produces one failed completion.
An internal send_active_ boolean separates delivery existence from byte
accounting in both transports. Existing queues, completion mechanisms and
source handling remain in use; no new queue, class, lock or thread was added.

**Acceptance and tests.** The original failure and four new tests pass on
Windows and WSL: isolated/consecutive empty deliveries, mixed output at the
byte limit, ordered cancellation after source failure, and stop/close draining
empty deliveries between body sources. Checks include null buffers, exact
completion counts and absence of callback reentry. The new isolated-delivery
test failed before the fix. The cancellation test permits a TCP reset after
a source failure, consistent with the existing local-write completion
contract; its callback order must still be success, success, failure, failure.
At B18 verification, full suites passed Windows unit 880/880, WSL unit
878/878 and integration 118/119 on both platforms; only B19 remained.
Clang ASan/UBSan with
leak detection passes 24 focused integration cases on Linux.

**Performance.** Three rotated WSL rounds use 8 workers, 4096 connections,
pipeline 1, separate CPU affinity, 5-second warmup and 15-second samples.
Both Doba builds use the same adapter/compiler flags and real 133-byte
responses with runtime Date. Median RPS: pre-B18 504,170, current
515,100, Actix 396,210. Current vs pre-B18: +2.17%;
vs Actix: +30.01%. Every measured response is 2xx. These WSL measurements
do not establish zero overhead or isolate the cause of observed differences.

**Limits.** The send budget counts bytes, not queue nodes. Empty deliveries
consume no byte budget; no new queue-entry limit was introduced. B19's
rejected-submission notification is resolved below.

**Components.** [Linux transport](../include/transport/server/tcpip_linux.h),
[Windows transport](../include/transport/server/tcpip_windows.h) and the
[output contract](../include/common/output.h).

| Suite | Test | Source | Windows / WSL |
| --- | --- | --- | --- |
| integration | tcpip removes an empty delivery without blocking its queue | [tests/integration/transport/server/tcpip_tests.cpp](../tests/integration/transport/server/tcpip_tests.cpp) | Passed |

<a name="b19-send-limit-failure-notification"></a>
<h3>B19: Missing completion notification for a rejected send</h3>

**Status.** Resolved; verified 2026-09-23. **Severity.** Critical.
**Priority.** P0 (maximum). **Target.** Current migration.

**Confirmed cause.** An invalid or oversized submission was rejected before
queue admission. Closure only counted accepted deliveries, so the rejected
block never produced on_send_completed(false).

**Agreed contract and implementation.** Each submission while the connection
is active completes once, including rejection. The first rejection starts
closure; later submissions are ignored. Both transports record a pending
rejection in send_rejected_, free rejected storage on return from send, and
append its failed completion to the existing cancellation accounting. The
flag is cleared before notification. Completion stays on the serialized I/O
worker path, after the engine callback and earlier deliveries. Windows waits
for active native I/O before retiring its buffers and notifying cancellation.
No new queue, class, thread, lock or allocation was introduced by the fix.

**Acceptance and tests.** The original B19 regression passes on Windows and
WSL unchanged. Three new tests cover isolated overflow, invalid buffers,
post-rejection submissions, an engine exception, queued deliveries, and an
in-flight body reader. They verify exact counts and no callback reentry.
The isolated-rejection test failed before the fix. The existing source-limit
test now expects two failures: cancellation of its active source and rejection
of the next block, as required by the approved contract. No test was disabled.
All suites pass: Windows unit 880/880, WSL unit 878/878, integration 122/122,
and helper suites 16/16 plus 11/11 on both platforms. Clang ASan/UBSan with
leak detection passes all 48 transport integration cases on Linux.

**Performance.** Three rotated WSL rounds use 8 workers, 4096 connections,
pipeline 1, separate CPU affinity, 5-second warmup and 15-second samples.
Identical Doba adapters/compiler flags use real 133-byte responses with
runtime Date. Median RPS: pre-B19 520,500, current 500,680,
Actix 423,990. Current vs pre-B19: -3.81%; vs Actix: +18.09%.
Every measured response is 2xx. These WSL measurements do not isolate the
cause of observed differences or establish zero overhead.

**Limits.** Completion reports local writing, failure or cancellation, not
peer receipt. Calls after closure starts remain ignored. Send-limit closure
and all unrelated transport behavior remain unchanged.

**Components.** [Linux transport](../include/transport/server/tcpip_linux.h),
[Windows transport](../include/transport/server/tcpip_windows.h) and the
[output contract](../include/common/output.h).

| Suite | Test | Source | Windows / WSL |
| --- | --- | --- | --- |
| integration | tcpip closes when a delivery exceeds the send limit | [tests/integration/transport/server/tcpip_tests.cpp](../tests/integration/transport/server/tcpip_tests.cpp) | Passed |

<a name="b9-response-driven-connection-close"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b9-response-driven-connection-close-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b9-response-driven-connection-close-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b9-response-driven-connection-close-dark.svg">
    <img src="../resources/docs/backlog/h3-b9-response-driven-connection-close.svg" alt="B9: Response-driven Connection close">
  </picture>
</h3>

**Status.** Resolved; verified 2026-09-23. **Severity.** Medium.
**Target.** 0.1.0-beta1.

**Confirmed behavior.** The old diagnosis described the architecture before
engines. The current engine already recognizes response Connection options,
including repeated fields, token lists and case differences. It submits the
response prefix and optional source, then requests closure. Both transports
drain admitted output before shutting down writing. Production code, public
contracts and build configuration were left unchanged.

**Acceptance.** RFC 9112 S9.6 requires completion of the closing response
before closure and no further request processing after that boundary.
New TCP tests cover synchronous, immediately completed coroutine and deferred
handlers with inline, streamed and chunked bodies. They verify complete
payloads, final chunk framing and EOF without successor bytes. A mixed
pipeline completes the closing handler before its predecessor, preserves
response order, discards an already prepared successor and leaves a queued
unsafe handler uncalled. The original deferred-close TCP test used a request
header and did not establish response-driven closure.

**Refactor coverage review.** Six unit and three integration cases were
added; existing assertions were strengthened without removing coverage.

| Contract | Evidence |
| --- | --- |
| Response-driven close | [engine_tests.cpp](../tests/unit/protocol/http/v11/engine_tests.cpp) checks immediate and queued delivery, later completion, cancellation, exact close tokens and request-close precedence. [server_response_tests.cpp](../tests/integration/protocol/http/v11/server_response_tests.cpp) verifies real TCP delivery before EOF. |
| Server, factories and policies | [server_engine_tests.cpp](../tests/unit/protocol/http/v11/server_engine_tests.cpp) adds six negative concept checks, independent decoder state, copied factory policy and effective per-engine policy forwarding. Existing tests cover shared routing and alternative transport policies. |
| Receive consumption | [tcpip_tests.cpp](../tests/integration/transport/server/tcpip_tests.cpp) verifies partial consumption of a full buffer, exact residual bytes, capacity and subsequent successful receives. Existing decoder and HTTP limit tests cover split input, full incomplete cores and oversized bodies streamed across receives. |
| Callback lifecycle | [tcpip_tests.cpp](../tests/integration/transport/server/tcpip_tests.cpp) now checks overlap across receive, wake and completion, no receive/wake after stop and exact completion counts. [tcpip_drain_tests.cpp](../tests/integration/transport/server/tcpip_drain_tests.cpp) adds one-stop-per-connection assertions to existing drain and admission tests. |
| Output ownership and draining | Existing engine tests verify transfer of unread sources; transport tests verify source/prefix FIFO, backpressure, empty deliveries, budget rejection, source failure, cancellation and drain on close, stop and destruction. |

**Validation.** Windows unit 886/886, WSL unit 884/884 and integration 125/125
on both platforms. Helper suites pass 16/16 and 11/11 on both. Builds include
examples. Clang ASan/UBSan with leak detection passes 44 affected unit and
60 integration cases on Linux. No test was disabled or skipped.

**Limits.** Safe handlers may already have run before a deferred response
reveals closure; their execution cannot be undone. Their later output is
not sent. Queued handlers are not dispatched after closure. Cancellation
remains cooperative. The decoder retains its last request through its
factory until replaced or destroyed; the new lifetime test checks release
after engine destruction, not immediately after stop. These tests do not
prove every possible concurrent interleaving. Windows sanitizers and a new
performance benchmark were not run; production sources are unchanged.

<a name="b13-signed-overflow-in-the-httparena-adapter"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-dark.svg">
    <img src="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter.svg" alt="B13: Signed overflow in the HttpArena adapter">
  </picture>
</h3>

**Status.** Runtime regression pending; medium severity, benchmark-only scope.

**Source evidence.** In
[main.cpp](../benchmarks/httparena/http/v11/main.cpp), `read_query_sum()` adds
two parsed `int64_t` values without a range check; the POST handler then adds
the parsed body value without one. Successful parsing of each operand does
not establish that either signed sum is representable.

**Acceptance and tests.** Exercise maximum plus one and minimum minus one
in both additions, with representable boundary sums and ordinary randomized
inputs as controls. Select a deterministic invalid-request result and verify
it with UBSan where available. Keep checks local to the adapter and preserve
the benchmark's expected arithmetic for valid inputs.

**Delivery.** Future work; not a `0.1.0-beta1` gate. Resolve before publishing
the affected adapter as verified.

<a name="product-and-convenience"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-product-and-convenience-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-product-and-convenience-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-product-and-convenience-dark.svg">
    <img src="../resources/docs/backlog/h2-product-and-convenience.svg" alt="Product and convenience">
  </picture>
</h2>

F1 and F2 are required for `0.1.0-beta1`, before operational hardening and
release engineering. P2-P8 remain future work, outside this beta.

<a name="f1-tls"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f1-tls-dark.svg">
    <img src="../resources/docs/backlog/h3-f1-tls.svg" alt="F1: TLS">
  </picture>
</h3>

**Release scope.** Integrate native TLS for `0.1.0-beta1` while preserving
the protocol-transport boundary. Original estimate: A.

**Decisions.** Select the TLS provider, dependency policy, configuration API
and ownership model in a separate implementation plan before coding.

**Acceptance and verification.** Focused unit and real-socket tests for
handshake, encrypted HTTP delivery, closure and errors, with equivalent
coverage on Windows and Linux.

**Delivery.** Mandatory release gate, ahead of C1/C2/C3/C7 and RE1.

<a name="f2-compression-and-gzip"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-f2-compression-and-gzip-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f2-compression-and-gzip-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f2-compression-and-gzip-dark.svg">
    <img src="../resources/docs/backlog/h3-f2-compression-and-gzip.svg" alt="F2: Compression and GZIP">
  </picture>
</h3>

**Release scope.** Add GZIP response compression for `0.1.0-beta1`, with
`Accept-Encoding`/`Content-Encoding` negotiation and `Vary` handling.
Other compression formats are outside this release scope.

**Decisions.** Select the compression library and API in a separate
implementation plan. Resolve the dependency policy and its impact on the
current zero-dependency claim before implementation.

**Acceptance and verification.** Focused unit and real-socket tests for
negotiation, uncompressed responses when identity is selected, consistent
`Vary` handling, correct framing and decoded payloads for empty, binary and
large bodies on Windows and Linux.

**Delivery.** Mandatory release gate, ahead of C1/C2/C3/C7 and RE1.

<a name="p2-access-logging"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p2-access-logging-dark.svg">
    <img src="../resources/docs/backlog/h3-p2-access-logging.svg" alt="P2: Access logging">
  </picture>
</h3>

**Context.** There is no dedicated access logging hook. The transport has
lifecycle callbacks, and the common logger is not a request log.

**Scope to define.** Decide the observation point and exposed data, including
when a response can be considered finished.

**Production API follow-up.** Identify the connection metadata required by
an access record, including the peer endpoint, status, duration and completion
outcome. Separate transport-observed addresses from forwarded client claims;
if forwarded metadata is supported, require an explicit trusted-proxy policy.
This is observation debt, not a requirement for a proxy/authentication
framework or a new 0.1 release gate.

**Components.** HTTP/transport composition, logger, tests, and examples.

**Proposed acceptance.** Check success, rejection, disconnection, and send
failure records without duplicates; define data lifetime and the cost when
logging is disabled.

**Dependencies.** Decide whether P3 is needed; do not introduce that dependency
by default or expose HTTP semantics inside the transport.

<a name="p3-middleware-chain"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-p3-middleware-chain-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p3-middleware-chain-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p3-middleware-chain-dark.svg">
    <img src="../resources/docs/backlog/h3-p3-middleware-chain.svg" alt="P3: Middleware chain">
  </picture>
</h3>

**Context.** Composing cross-cutting logic currently requires repeating it
in handlers. The design must respect the framework's lightweight approach.

**Scope to define.** First resolve concrete composition use cases and their
interaction with synchronous and deferred handlers.

**Components.** Route registration API, handler contracts, and tests.

**Proposed acceptance.** Specify ordering, short-circuiting, errors,
cancellation, and ownership; test them without penalizing the path without
middleware.

**Out of scope.** No middleware architecture has been decided, and there is
no justification for introducing a general extension framework.

<a name="p4-form-parsing"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p4-form-parsing-dark.svg">
    <img src="../resources/docs/backlog/h3-p4-form-parsing.svg" alt="P4: Form parsing">
  </picture>
</h3>

**Context.** Neither `application/x-www-form-urlencoded` nor
`multipart/form-data` parsing exists. A percent-decoding primitive is
available. Original estimate: M.

**Scope to define.** Separate the two formats and decide how to represent
repeated fields, binary payloads, and errors without assuming that their
grammars match query syntax.

**Components.** HTTP helpers, body reader, form API, and tests.

**Proposed acceptance.** Valid and malformed cases, encoding, empty/repeated
fields, limits, and fragmented boundaries for multipart.

**Dependencies.** Reuse existing primitives only where their semantics match.
Coordinate resource budgets with C2.

<a name="p5-automatic-conditionals-and-ranges"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges-dark.svg">
    <img src="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges.svg" alt="P5: Automatic conditionals and ranges">
  </picture>
</h3>

**Remaining scope.** Define optional helpers that evaluate application-supplied
entity-tag and modification-date validators and, if offered, select byte
ranges and construct partial-content responses. The framework must not invent
metadata for the selected representation.

**Evidence.** [static_file_server.h](../include/protocol/http/v11/static_file_server.h)
handles existence-based `If-Match: *` and `If-None-Match: *`, and rejects
explicit If-Match tags because it supplies no entity tags. Entity-tag/date
validators and ranges remain absent from that handler. Status factories and
conditional-field parsing are available; they do not require reimplementation.

**Components.** Conditional evaluation helpers, HTTP API, static-file
integration where applicable, and focused examples and tests.

**Acceptance and tests.** Define the metadata contract, test precondition
precedence for the added validators, and cover selected ranges and unsatisfied
ranges if supported. Preserve the existing wildcard conditions and the
ability to ignore Range and serve a normal GET.

**References.** RFC 9110 S13.2, S13.2.2, and S14.

**Delivery.** Future work; neither a core compliance gate nor a release gate.

<a name="p6-output-trailers"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p6-output-trailers-dark.svg">
    <img src="../resources/docs/backlog/h3-p6-output-trailers.svg" alt="P6: Output trailers">
  </picture>
</h3>

**Context.** `response` exposes no API for emitting trailers.

**Future scope.** Define an API limited to framing that supports trailers,
with field validation and consistent body finalization.

**Components.** Response, output writers, and their tests.

**Acceptance and tests.** Verify the complete wire output, terminator, and
trailer field restrictions; preserve output without trailers.

**Reference.** RFC 9110 S6.5.

**Delivery.** Future work; not a release gate.

<a name="p7-automatic-resource-options"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-p7-automatic-resource-options-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p7-automatic-resource-options-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p7-automatic-resource-options-dark.svg">
    <img src="../resources/docs/backlog/h3-p7-automatic-resource-options.svg" alt="P7: Automatic resource OPTIONS">
  </picture>
</h3>

**Context.** `OPTIONS *` is already automatic. The application can register
`OPTIONS` handlers for specific resources.

**Future scope.** Synthesize responses through the router by reusing the
`Allow` calculation used by `405`.

**Components.** Router, HTTP server, and route tests.

**Acceptance and tests.** Preserve static, parameterized, and wildcard route
precedence; define precedence for an explicit handler and test existing and
missing resources.

**Delivery.** Future work; not a release gate.

<a name="p8-429-response-construction"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-p8-429-response-construction-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p8-429-response-construction-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p8-429-response-construction-dark.svg">
    <img src="../resources/docs/backlog/h3-p8-429-response-construction.svg" alt="P8: 429 response construction">
  </picture>
</h3>

**Context.** The response API provides named status factories but no 429
factory, while the general status constructor is not public. Applications
implementing their own rate or admission policy need a supported way to
return this status without manually rewriting serialized output.

**Components.** [response.h](../include/protocol/http/v11/response.h), status
definitions, response unit tests and a focused usage example if needed.

**Scope to decide.** Prefer the smallest addition consistent with current
factory conventions. A generic status API is a separate compatibility
decision, not required by this item. Do not add built-in rate limiting.

**Acceptance and tests.** Verify the 429 status line, optional application-set
Retry-After, body framing and HEAD behavior. Preserve existing factories.

**Delivery.** Future work; not a `0.1.0-beta1` gate.

<a name="quality-and-validation"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-quality-and-validation-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-quality-and-validation-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-quality-and-validation-dark.svg">
    <img src="../resources/docs/backlog/h2-quality-and-validation.svg" alt="Quality and validation">
  </picture>
</h2>

QA1-QA3 and QA5-QA6 are future work. The existing CI gates and focused
tests for B9, F1/F2 and C1/C2/C3/C7 remain mandatory for `0.1.0-beta1`;
completing these broader QA programs is not required.

<a name="qa1-exhaustive-compliance-suite"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite-dark.svg">
    <img src="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite.svg" alt="QA1: Exhaustive compliance suite">
  </picture>
</h3>

**Remaining work.** Establish a requirement-to-test map and extend coverage
only where an HTTP rule, fragmentation point, or transport boundary remains
unverified. Trace new cases to their applicable RFC requirements and verify
equivalent results in IOCP and epoll.

**Concrete gaps.**

- Force a collision with an actual spill filename candidate; verify existing
  file preservation, retry, error classification, and cleanup. The
  [storage tests](../tests/unit/common/byte_storage_tests.cpp) preserve an
  unrelated `existing.tmp`, which does not force that collision.
- Prove that output remains pending while another client is served. The
  [transport test](../tests/integration/transport/server/tcpip_tests.cpp)
  observes serialization before serving the second client, without proving
  a send remains blocked. Also test the absolute `send_all` deadline against
  a non-reading local peer.
- Reproduce and resolve the gap between `find_available_port()` releasing its
  socket and server bind in the
  [client helper](../tests/integration/helpers/tcpip_client.h); distinguish
  address-in-use from unrelated startup failures.
- Exercise a genuinely pending connect with a deterministic local mechanism
  on Windows and Linux. The
  [client tests](../tests/integration/helpers/tcpip_client_tests.cpp) cover
  refusal and receive deadlines, not that connect timeout.

**Components.** Decoder and framing tests, storage tests, protocol-transport
integration, and test helpers. Reuse C2 boundary tests and C7 pending-response
measurements when this future work resumes; those focused tests belong to
the release items and must not wait for QA1 or QA5.

**Acceptance.** Record the rule or contract and missing boundary exercised by
each new case. A passing test count is not an exhaustive compliance claim.

<a name="qa2-fuzzing"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa2-fuzzing-dark.svg">
    <img src="../resources/docs/backlog/h3-qa2-fuzzing.svg" alt="QA2: Fuzzing">
  </picture>
</h3>

**Remaining work.** Prepare a fuzzing plan for helpers, the decoder,
framers/readers, request rebasing, serialization, byte storage, and relevant
transport boundaries. No fuzz targets or corpus are present in the project.

**Proposed acceptance.** Define targets, an initial corpus, budgets, and
failure reproduction. Preserve every confirmed failure as a deterministic
regression and run targets with the appropriate sanitizer.

**Dependencies and limits.** Select infrastructure before adding
dependencies. Fuzzing integration remains deferred beyond the current
hardening effort.

<a name="qa3-performance-baseline"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-qa3-performance-baseline-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa3-performance-baseline-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa3-performance-baseline-dark.svg">
    <img src="../resources/docs/backlog/h3-qa3-performance-baseline.svg" alt="QA3: Performance baseline">
  </picture>
</h3>

**Remaining work.** Turn the published HttpArena measurements into a repeatable
baseline for future releases, with regression tolerances. Pin the Web
Frameworks runner, whose adapter currently references an inspected branch
rather than a commit.

**Evidence.** The [published results](../resources/benchmarks/benchmark-results.txt)
record runner and doba revisions, throughput, latency, CPU, and memory for
baseline and pipelined profiles, with one repetition. These measurements do
not establish run-to-run variability, allocation counts, or scaling limits.

**Scope.** Record hardware, toolchain, revisions, scenarios, and conditions
for minimal requests, large headers, inline/streaming bodies, pipelining,
and concurrency. Repeat samples, cover the missing resource metrics, and
select representative loads for release comparison.

**Components.** Benchmark adapters and result collection for the decoder,
router, serialization, and transports.

**Acceptance.** Define tolerances from measured noise and preserve enough
inputs and results to reproduce a comparison with the release candidate.
Keep measurement separate from optimization.

<a name="qa5-stress-campaigns"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-qa5-stress-campaigns-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa5-stress-campaigns-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa5-stress-campaigns-dark.svg">
    <img src="../resources/docs/backlog/h3-qa5-stress-campaigns.svg" alt="QA5: Stress campaigns">
  </picture>
</h3>

**Status:** pending. **Priority:** not set. **Target:** future work.

**Context.** Functional integration and concurrency cases are in place.
Extended soak tests and, where feasible, controlled worker interleavings
remain to be explored. Define duration, load, observed resources, and
reproduction before creating new tests; relate them to C1/C3 and QA3 scenarios.

<a name="qa6-external-compliance-automation"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-qa6-external-compliance-automation-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa6-external-compliance-automation-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa6-external-compliance-automation-dark.svg">
    <img src="../resources/docs/backlog/h3-qa6-external-compliance-automation.svg" alt="QA6: External compliance automation">
  </picture>
</h3>

**Status:** deferred. **Priority:** not set. **Target:** future work.

**Context.** h1spec and Http11Probe are run manually. Their automation is
deferred beyond the current hardening effort. When resumed, use the versioned
adapters, pin the environment, and distinguish runner failures from protocol
failures. Preserve logs and revisions for each execution. This task
complements QA1.

<a name="release-engineering"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-release-engineering-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-release-engineering-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-release-engineering-dark.svg">
    <img src="../resources/docs/backlog/h2-release-engineering.svg" alt="Release engineering">
  </picture>
</h2>

<a name="re1-release-governance-and-traceability"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-re1-release-governance-and-traceability-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-re1-release-governance-and-traceability-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-re1-release-governance-and-traceability-dark.svg">
    <img src="../resources/docs/backlog/h3-re1-release-governance-and-traceability.svg" alt="RE1: Release governance and traceability">
  </picture>
</h3>

**Release scope.** Complete the minimum closure for `0.1.0-beta1`: release
notes or changelog, a security policy with an available vulnerability-reporting
channel, minimum usage documentation, and coherent versioning and publication.
The [publishing workflow](../.github/workflows/ci.yml) currently derives only
`vMAJOR.MINOR.PATCH` from `include/version.h`, without a prerelease suffix.
Select a mechanism that publishes the agreed beta identifier consistently.

**Components.** Public project documents, version, and release procedure;
modify the workflow only if the selected mechanism requires it.

**Acceptance and verification.**

- Retain the verified B9 regressions and complete F1/F2 and C1/C2/C3/C7
  with focused tests and equivalent real-socket validation on Windows and
  Linux where applicable.
- Pass the full existing CI matrix and CMake consumer checks on the exact
  release revision. Include every required source, test and example in that
  revision; local untracked files are not part of a published release.
- Document current indexed-getter preconditions, request/view and borrowed
  storage lifetimes, manual response-framing responsibilities and header
  capacity, and allowed lifecycle/callback operations. This minimum does not
  require completing DT2/DT3/DOC1/DOC2 or changing their APIs.
- Describe the controlled-deployment scope, TLS and GZIP configuration,
  effective operational policies, known limitations and omitted capabilities.
  Publish only channels and procedures that are actually available.
- Align version metadata, tag `v0.1.0-beta1`, release notes and documentation
  with the tested content. Publish the beta as a prerelease.

**Future work.** The full contribution guide and broader contribution and
compatibility procedures remain outstanding here, outside the beta gate.
They must not be mistaken for completed work when the minimum release closure
is delivered. The prerelease publishing mechanism remains a design decision,
not an authorization for a particular workflow modification.

<a name="c-maintainability"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-c-maintainability-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-c-maintainability-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-c-maintainability-dark.svg">
    <img src="../resources/docs/backlog/h2-c-maintainability.svg" alt="C++ maintainability">
  </picture>
</h2>

DT1-DT3 are future work. RE1 covers the minimum documentation of current
usage preconditions for `0.1.0-beta1`; these entries do not require API
changes or a broader redesign before that release.

<a name="dt1-platformh-dependencies-and-global-effects"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-dt1-platformh-dependencies-and-global-effects-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-dt1-platformh-dependencies-and-global-effects-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-dt1-platformh-dependencies-and-global-effects-dark.svg">
    <img src="../resources/docs/backlog/h3-dt1-platformh-dependencies-and-global-effects.svg" alt="DT1: platform.h dependencies and global effects">
  </picture>
</h3>

**Remaining work.** Review the remaining consumer-visible effects of
[platform.h](../include/platform.h): Windows macros, warning 4996 suppression,
linker pragmas, and transitive standard/system includes. Decide the diagnostic
contract for unsupported platforms; the
[TCP selector](../include/transport/server/tcpip.h) currently has no explicit
unsupported-platform branch.

**Acceptance and verification.** Inventory and justify the remaining
macros/pragmas and dependencies. Check other public headers independently
before proposing a dependency fix. Verify that any localized change preserves
consumer compiler options and installed-package usage on supported compilers.

**Limits.** Only evidence-backed local changes are in scope. No general
include reorganization, expansion of supported platforms, or automatic
release gate is implied.

<a name="dt2-indexed-getter-contract"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-dt2-indexed-getter-contract-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-dt2-indexed-getter-contract-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-dt2-indexed-getter-contract-dark.svg">
    <img src="../resources/docs/backlog/h3-dt2-indexed-getter-contract.svg" alt="DT2: Indexed getter contract">
  </picture>
</h3>

**Context.** `request::get_header(size_t)` and
`get_query_parameter(size_t)` use `operator[]`; callers must supply a valid
index. Other APIs use exceptions or optional for absence.

**Risk.** An invalid index causes undefined behavior when that precondition
is violated.

**Components.** Request API, tests, and public documentation.

**Acceptance and verification.** Decide whether index validity is a
documented precondition or requires a check. Specify the contract, test
boundaries according to the decision, and preserve compatibility.
Measure the impact if frequent iterations are affected.

**Dependencies.** Coordinate with the following view and getter documentation.
Inconsistency alone does not authorize an API change.

<a name="dt3-response-framing-and-capacity-contract"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract-dark.svg">
    <img src="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract.svg" alt="DT3: Response framing and capacity contract">
  </picture>
</h3>

**Status.** Decision pending; this is not a confirmed misuse-independent bug.

**Open contract.** In [response.h](../include/protocol/http/v11/response.h),
manual Content-Length or Transfer-Encoding can override deferred body framing.
Decide whether mismatched lengths and manually declared chunked encoding are
caller preconditions or require validation. Specify supported combinations,
operation order, framing ownership, and failure behavior.

**Remaining coverage.** Add shorter/longer explicit-length cases and manual
Transfer-Encoding cases according to the selected contract, checking complete
wire output. Preserve the
[existing tests](../tests/unit/protocol/http/v11/response_tests.cpp) for matching
lengths, conflicting Content-Length/Transfer-Encoding, HEAD/bodyless responses,
and exact capacity with a fixed Date and deferred Content-Length.

**Capacity follow-up.** Document the fixed status/header budget and the space
needed for generated fields. Add exact-fit and over-capacity cases combining
automatic Date with deferred framing, and verify recovery cannot publish a
partial malformed response. Preserve method-aware error recovery;
keep header capacity distinct from body storage and progressive streaming.

**Delivery and limits.** Future work. RE1 documents current response
preconditions for the beta; the contract changes and extra coverage above
remain outside its gate. No dynamic buffers, ownership redesign, broad
serializer refactor, or public API change is prescribed.

<a name="public-documentation"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-public-documentation-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-public-documentation-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-public-documentation-dark.svg">
    <img src="../resources/docs/backlog/h2-public-documentation.svg" alt="Public documentation">
  </picture>
</h2>

DOC1-DOC2 are future work for extended guides and examples. Minimum usage
preconditions and deployment limitations belong to the RE1 release closure.

<a name="doc1-transport-lifecycle"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-doc1-transport-lifecycle-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-doc1-transport-lifecycle-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-doc1-transport-lifecycle-dark.svg">
    <img src="../resources/docs/backlog/h3-doc1-transport-lifecycle.svg" alt="DOC1: Transport lifecycle">
  </picture>
</h3>

**Outstanding work.** Add practical documentation and public examples for
direct transport use outside `v11::server`: callback setup, start/stop order,
allowed calling threads, restart, and startup errors.

**Acceptance.** Document the worker-thread restriction on `stop()` and when
callbacks may be changed. Distinguish per-connection graceful closure and
fatal abort from the server-wide draining proposed in F7. Link the guide
from architecture and validate examples against the
[transport lifecycle tests](../tests/integration/transport/server/tcpip_tests.cpp).

<a name="doc2-request-views-and-getters"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-doc2-request-views-and-getters-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-doc2-request-views-and-getters-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-doc2-request-views-and-getters-dark.svg">
    <img src="../resources/docs/backlog/h3-doc2-request-views-and-getters.svg" alt="DOC2: Request views and getters">
  </picture>
</h3>

**Outstanding work.** Extend the architecture's ownership overview with
concrete examples of retaining request views safely, destruction-related
invalidation, borrowed-reader lifetime, and indexed getter preconditions.

**Acceptance.** Show valid uses and explain why views cannot outlive their
owning request or external storage. Document the indexed-access contract
selected in DT2, cross-check the
[request tests](../tests/unit/protocol/http/v11/request_tests.cpp) and reader
tests, and link the contract from the corresponding examples.

<a name="beyond-the-first-release"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-beyond-the-first-release-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-beyond-the-first-release-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-beyond-the-first-release-dark.svg">
    <img src="../resources/docs/backlog/h2-beyond-the-first-release.svg" alt="Beyond the first release">
  </picture>
</h2>

F3-F7 are future work, outside `0.1.0-beta1`. F1/F2 are release gates.
C1/C2/C3/C7 and their focused slow-client and resource-limit tests remain
required for the beta. The broader QA3 performance baseline and QA5 stress
campaigns remain future work.

<a name="f3-progressive-streaming-and-sse"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse-dark.svg">
    <img src="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse.svg" alt="F3: Progressive streaming and SSE">
  </picture>
</h3>

**Context.** The handler finishes producing the body before handing off the
response; current draining does not constitute progressive streaming.
Original estimate: A.

**Future scope.** Represent start, fragments, and completion/error; apply
byte-based backpressure and batch small fragments.

**Proposed acceptance.** Test cancellation, slow clients, partial errors,
and ordering. Preserve the cost of the one-shot path and synchronous hot
path by comparing against QA3.

<a name="f4-ordered-upgrade-barrier"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier-dark.svg">
    <img src="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier.svg" alt="F4: Ordered upgrade barrier">
  </picture>
</h3>

**Context.** The `101` must reach the head of the response order before
transferring the channel. Original estimate: A.

**Future scope.** Stop HTTP decoding at the correct point, deliver residual
bytes to the new codec, and transfer control.

**Proposed acceptance.** Test first with a dummy codec: previous responses,
already received bytes, closure, and errors. The contract remains generic
and does not leak HTTP semantics into IOCP or epoll.

**Dependencies.** Prerequisite for F5 (WebSockets).

<a name="f5-websockets"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f5-websockets-dark.svg">
    <img src="../resources/docs/backlog/h3-f5-websockets.svg" alt="F5: WebSockets">
  </picture>
</h3>

**Context.** `channel_intent::kUpgrade` is defined and Sec-WebSocket-*
headers are modeled, but transports do not handle the upgrade.
The first release does not claim this capability. Original estimate: A.

**Future scope.** Handshake, framing, fragmentation, ping/pong/close,
externally initiated sends, and bidirectional backpressure.

**Proposed acceptance.** First define the protocol contract and compliance;
test fragmented messages, simultaneous closure, errors, and slow clients
on both platforms.

**Dependencies.** F4 (ordered upgrade barrier). Preserve what is already
modeled; deferring this feature does not by itself make the core
noncompliant with HTTP/1.1.

<a name="f6-listener-and-worker-configuration"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-f6-listener-and-worker-configuration-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f6-listener-and-worker-configuration-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f6-listener-and-worker-configuration-dark.svg">
    <img src="../resources/docs/backlog/h3-f6-listener-and-worker-configuration.svg" alt="F6: Listener and worker configuration">
  </picture>
</h3>

**Context.** The current TCP backends bind the IPv4 wildcard address and
derive worker count from hardware concurrency. Applications cannot select
a narrower listening address or a worker budget through the normal server
configuration API.

**Future scope.** Decide whether deployment requirements justify minimal
bind-address and worker-count options, following the module-creation policy
contracts selected for C1/C2/C3/C7. Do not introduce a general networking
abstraction or expand supported address families by default.

**Components and acceptance.** Server configuration and both TCP backends.
Test loopback-only binding, invalid or unavailable addresses, default behavior,
worker validation, startup failure cleanup and stop on both platforms.

**Delivery.** Future work; record current deployment constraints without
presenting these options as existing features. This is separate from C3's
connection admission budget and requires no changes to that release target.

<a name="f7-shutdown-with-draining"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-f7-shutdown-with-draining-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f7-shutdown-with-draining-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f7-shutdown-with-draining-dark.svg">
    <img src="../resources/docs/backlog/h3-f7-shutdown-with-draining.svg" alt="F7: Shutdown with draining">
  </picture>
</h3>

**Context.** The current stop lifecycle is not a public bounded-drain API
for completing all accepted work during deployment shutdown. Graceful closure
of an individual connection is not the same guarantee as server-wide draining.

**Future scope.** Specify when acceptance and new request admission stop,
what happens to active and deferred handlers, and how a drain deadline ends
remaining work. Preserve existing `stop()` semantics unless an explicit
compatibility decision authorizes a change.

**Components and acceptance.** Server lifecycle, both TCP backends and
lifecycle integration tests. Cover active sends, idle keep-alive, pipelining,
pending deferred work, deadline expiry and repeated stop; verify bounded
completion and single cleanup on each platform.

**Delivery.** Future work, outside `0.1.0-beta1`. Until implemented,
document abrupt-stop limitations and verify any external deployment-draining
procedure before relying on it. Coordinate public lifecycle documentation
with DOC1; do not count documentation alone as implementation.
