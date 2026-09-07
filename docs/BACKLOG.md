# Backlog

[Index](HANDOFF.md)

Source of truth for doba's outstanding work. Each category has consecutive
numbering, and each item has a single entry. Previous identifiers are
preserved in the [identifier mapping](#identifier-mapping).
Engineering and verification requirements are defined in
[QUALITY.md](QUALITY.md).

An outstanding item does not imply that its design has been decided.
The criteria below help prepare implementation; open decisions are stated
explicitly. Priorities without a previous value are marked "Not set".
The original B/M/A labels estimated complexity and are not reinterpreted
as severity.

## Contents

- [Release target](#release-target)
- [Inventory](#inventory)
- [Operational hardening](#operational-hardening)
- [Product and convenience](#product-and-convenience)
- [Quality and validation](#quality-and-validation)
- [Release engineering](#release-engineering)
- [C++ maintainability](#c-maintainability)
- [Public documentation](#public-documentation)
- [Beyond the first release](#beyond-the-first-release)
- [Identifier mapping](#identifier-mapping)

## Release target

The current `0.1.0-beta.1` target is to complete C1 and C3 and verify them
over real sockets on Windows and Linux. Release also requires preserving
CI and CMake consumer validations and completing the remainder of RE1.

Recommended order:

1. C1: a single inactivity timeout.
2. C3: a global active connection limit.
3. Close RE1 and verify the complete release matrix.

C1 and C3 share generic configuration before `start()` and platform-specific
execution in IOCP and epoll. Define the common part before addressing each
localized change. Default values and the exact API shape have not yet
been selected.

C2, fuzzing, and external tool automation are explicitly deferred beyond
the current hardening effort. P5-P7 are neither compliance nor release gates.
DT1 and DT2 do not automatically block the first release either.
Other outstanding items have no assigned version.

## Inventory

26 items across seven categories. Numbering identifies items; it does not
express priority or implementation order.

| Category | Identifiers | Total |
| --- | --- | --- |
| Operational hardening | C1-C3 | 3 |
| Product and convenience | P1-P7 | 7 |
| Quality and validation | QA1-QA6 | 6 |
| Release engineering | RE1 | 1 |
| C++ maintainability | DT1-DT2 | 2 |
| Public documentation | DOC1-DOC2 | 2 |
| Beyond the first release | F1-F5 | 5 |
| **Total** | | **26** |

| Item | Category | Status | Priority | Target |
| --- | --- | --- | --- | --- |
| [C1](#c1-single-inactivity-timeout) | Hardening | Pending | Beta target | 0.1.0-beta.1 |
| [C2](#c2-effective-per-request-limits) | Hardening | Deferred | High | No assigned version |
| [C3](#c3-global-active-connection-limit) | Hardening | Pending | Beta target | 0.1.0-beta.1 |
| [P1](#p1-static-file-handler) | Product | Pending | Not set | No assigned version |
| [P2](#p2-access-logging) | Product | Pending | Not set | No assigned version |
| [P3](#p3-middleware-chain) | Product | Pending | Not set | No assigned version |
| [P4](#p4-form-parsing) | Product | Pending | Not set | No assigned version |
| [P5](#p5-automatic-conditionals-and-ranges) | Product | Deferred | Not set | No assigned version |
| [P6](#p6-output-trailers) | Product | Deferred | Not set | No assigned version |
| [P7](#p7-automatic-resource-options) | Product | Deferred | Not set | No assigned version |
| [QA1](#qa1-exhaustive-compliance-suite) | QA | Pending | Not set | No assigned version |
| [QA2](#qa2-fuzzing) | QA | Deferred | High | No assigned version |
| [QA3](#qa3-performance-baseline) | QA | Pending | Medium | No assigned version |
| [QA4](#qa4-harness-diagnostics-and-isolation) | QA | Pending | Medium | No assigned version |
| [QA5](#qa5-stress-campaigns) | QA | Pending | Not set | No assigned version |
| [QA6](#qa6-external-compliance-automation) | QA | Deferred | Not set | No assigned version |
| [RE1](#re1-release-governance-and-traceability) | Release | Partial | Medium | 0.1.0-beta.1 |
| [DT1](#dt1-platformh-dependencies-and-global-effects) | C++ | Pending | Medium | No assigned version |
| [DT2](#dt2-indexed-getter-contract) | C++ | Pending | Low/Medium | No assigned version |
| [DOC1](#doc1-transport-lifecycle) | Documentation | Pending | Not set | No assigned version |
| [DOC2](#doc2-request-views-and-getters) | Documentation | Pending | Not set | No assigned version |
| [F1](#f1-tls) | Future | Deferred | Not set | Beyond 0.1 |
| [F2](#f2-compression-and-gzip) | Future | Deferred | Not set | Beyond 0.1 |
| [F3](#f3-progressive-streaming-and-sse) | Future | Deferred | Not set | Beyond 0.1 |
| [F4](#f4-ordered-upgrade-barrier) | Future | Deferred | Not set | Beyond 0.1 |
| [F5](#f5-websockets) | Future | Deferred | Not set | Beyond 0.1 |

## Operational hardening

### C1: Single inactivity timeout

**Context.** Neither backend currently closes an open connection that stops
making progress. Original complexity estimate: M.

**Scope.** A single `inactivity_timeout`, configurable before `start()`,
applies to reads, keep-alive, and writes. Receiving or sending bytes renews
the deadline; the total duration of a connection or request does not exhaust
it while progress continues. On expiry, the transport closes safely.

**Components.** Transport configuration, `tcpip_windows.h`,
`tcpip_linux.h`, and TCP/IP integration tests.

**Acceptance and tests.**

- A fragmented read that makes progress keeps the connection alive.
- A stalled partial request, idle keep-alive, and a client that does not read
  cause closure when the applicable deadline expires.
- Check timing, lifetime, ordering, and exactly one callback per closure.
- Run equivalent scenarios in IOCP and epoll.

**Dependencies and decisions.** Share generic configuration with C3.
Determine the API, default value, possible disabling behavior, and test timing
tolerance before implementation.

**Out of scope.** Separate deadlines per phase, an absolute maximum duration,
dynamic configuration, and automatic `408` responses.

### C2: Effective per-request limits

**Context.** `max_content_length`, `max_forwarding_hops`,
`max_transfer_codings`, `max_uri_length`, and `max_header_section_size`
use zero for unlimited. The default server exposes no configuration.
The internal buffer bounds the head but does not provide a general resource
policy. Bodies can spill to disk.

**Future scope.** Define safe defaults and a minimal API before `start()`.
Reject excessive Content-Length before consuming the body. Separately check
the policy for chunked bodies, whose size is not known in advance.

**Components.** `policies.h`, `limits.h`, `context.h`, `decoder.h`,
header rules, server composition, and their tests.

**Acceptance and tests.**

- Document what each value limits and how it applies in the standard server.
- Test the exact limit, one unit above it, and zero semantics.
- Check early rejection and the corresponding rejection reason.
- Verify that configuration cannot change unsafely during use.
- Measure whether the change affects the hot path.

**Dependencies and decisions.** Requires a dedicated API and defaults design.
Explicitly deferred; the current plan does not change these policies or their
consumers. Do not confuse this with C3.

### C3: Global active connection limit

**Context.** `connections_` is observational and does not limit admission.
Original complexity estimate: M.

**Scope.** A global maximum configured before `start()`. Each backend
atomically reserves capacity before admitting a context. Once the limit is
reached, it immediately closes the new connection while preserving those
already admitted.

**Components.** Generic configuration, admission and closure in both backends,
the connection counter, and TCP/IP tests.

**Acceptance and tests.**

- Exceed the maximum with concurrent clients without exceeding capacity.
- Previously admitted connections remain served.
- Every closure releases exactly one reservation, including error paths.
- A new connection can enter once capacity is available again.
- Verify equivalent behavior on Windows and Linux.

**Dependencies and decisions.** Coordinate with C1; determine defaults and
the semantics of any unlimited value. C3 controls connections; C2 controls
resources per request.

**Out of scope.** Per-worker quotas, dynamic changes, acceptance backpressure,
new callbacks, and HTTP rejection responses.

## Product and convenience

### P1: Static file handler

**Context.** Output body draining and percent-decoding already exist.
The latter is a security dependency. Original estimate: M.

**Scope to define.** A file handler using those primitives.
`TransmitFile`/`sendfile` are options to evaluate through measurements,
not a decided architectural requirement.

**Components.** HTTP handler, path resolution, output body, and examples.

**Proposed acceptance.** Define the allowed root, access errors, and file
lifetime; verify encoded paths, traversal, missing files, binary bodies,
and large output with bounded memory.

**Dependencies.** Preserve the generic boundary if a platform-specific send
path is introduced. This does not assume automatic range support.

### P2: Access logging

**Context.** There is no dedicated access logging hook. The transport has
lifecycle callbacks, and the common logger is not a request log.

**Scope to define.** Decide the observation point and exposed data, including
when a response can be considered finished.

**Components.** HTTP/transport composition, logger, tests, and examples.

**Proposed acceptance.** Check success, rejection, disconnection, and send
failure records without duplicates; define data lifetime and the cost when
logging is disabled.

**Dependencies.** Decide whether P3 is needed; do not introduce that dependency
by default or expose HTTP semantics inside the transport.

### P3: Middleware chain

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

### P4: Form parsing

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

### P5: Automatic conditionals and ranges

**Context.** Doba preserves conditional fields; the handler has the resource
validators. The application, when acting as an origin server, is responsible
for evaluating preconditions. Range support is optional.

**Future scope.** Helpers for `304`, `412`, `206`, or `416`, if they
are offered. The framework must not invent metadata for the selected
representation.

**Components.** HTTP API, conditional headers, response, and examples.

**Acceptance and tests.** Define the contract for application-provided
validators, test precondition precedence, and preserve the ability to ignore
Range and serve a normal GET.

**References.** RFC 9110 S13.2, S13.2.2, and S14.

**Out of scope.** This is neither a core compliance gate nor a release gate.
Invalid conditional date handling is already implemented.

### P6: Output trailers

**Context.** `response` exposes no API for emitting trailers.

**Future scope.** Define an API limited to framing that supports trailers,
with field validation and consistent body finalization.

**Components.** Response, output writers, and their tests.

**Acceptance and tests.** Verify the complete wire output, terminator, and
trailer field restrictions; preserve output without trailers.

**Reference.** RFC 9110 S6.5.

**Out of scope.** Optional capability with no assigned release gate.

### P7: Automatic resource OPTIONS

**Context.** `OPTIONS *` is already automatic. The application can register
`OPTIONS` handlers for specific resources.

**Future scope.** Synthesize responses through the router by reusing the
`Allow` calculation used by `405`.

**Components.** Router, HTTP server, and route tests.

**Acceptance and tests.** Preserve static, parameterized, and wildcard route
precedence; define precedence for an explicit handler and test existing and
missing resources.

**Out of scope.** Optional convenience; not a release gate.

## Quality and validation

### QA1: Exhaustive compliance suite

**Context.** The project's integration suite covers framing, fragmentation,
pipelining, and closure over real sockets. It is not an exhaustive RFC matrix.

**Risk.** A regression can depend on TCP segmentation, concatenated requests,
limits, or ambiguous framing and escape existing cases.

**Scope.** Systematically expand positive and negative cases for request
smuggling, Content-Length/Transfer-Encoding, chunked encoding, trailers,
and limits.

**Components.** Decoder, framing rules, request/response,
protocol-transport contract, and both backends.

**Acceptance and tests.** Trace each group of cases to its RFC rule, cover
relevant fragmentation points, and verify equivalent results in IOCP and
epoll. Configured limit cases depend on the C2 decisions.

**References.** RFC 9110 and RFC 9112. The suite provides evidence for the
strict HTTP/1.1 claim; it does not replace contract review.

### QA2: Fuzzing

**Context.** ASan, UBSan, and TSan are already integrated. Written tests execute
a finite set of paths.

**Future scope.** Prepare a separate fuzzing plan for helpers, the decoder,
framers/readers, request rebasing, serialization, byte storage, and relevant
transport boundaries.

**Proposed acceptance.** Define targets, an initial corpus, budgets, and
failure reproduction. Preserve every confirmed failure as a deterministic
regression and run targets with the appropriate sanitizer.

**Dependencies and limits.** Select infrastructure before adding
dependencies. CI fuzzing integration is deferred beyond the current hardening
effort; do not reopen the completed sanitizer work.

### QA3: Performance baseline

**Context.** Adapters exist for HttpArena and Web Frameworks Benchmark.
A persistent baseline and a gate for throughput, latency, allocations,
memory, and scaling are missing. The upstream Web Frameworks runner has no
pinned commit.

**Impact.** The high-performance claim cannot be quantified, and decisions
about shared_ptr, std::function, spill, or buffers cannot be compared with
confidence.

**Scope.** Pin revisions, scenarios, and conditions for minimal requests,
large headers, inline/streaming bodies, pipelining, and concurrency.

**Components.** Adapters, decoder, router, serialization, and transports.

**Acceptance and tests.** Record the toolchain, hardware, exact revisions,
latency distributions, and repeated samples. Define tolerances based on
measured noise and a reproducible comparison with the release.

**Dependencies.** Separate measurement from optimization. Individual
optimization measurements do not replace this release baseline.

### QA4: Harness diagnostics and isolation

**Context.** The unit runner invokes each case without catching exceptions.
An unexpected exception can terminate the executable. `expect` receives
the expression, file, and line but does not print them. CTest registers the
entire unit suite as a single test.

**Impact.** Intermittent or exceptional failures provide little diagnostic
information and can hide the results of later cases.

**Components.** `tests/unit/test_helper.h`, `test_helper.cpp`, and CTest
registration; inspect the integration helper if it shares this behavior.

**Acceptance and tests.** A throwing case must be identified as failed and
allow subsequent cases to be reported. Each failed assertion shows its
expression and location. Evaluate CTest granularity without unnecessary
dependencies.

### QA5: Stress campaigns

**Status:** pending. **Priority:** not set. **Target:** no assigned version.

**Context.** Functional integration and concurrency coverage is complete.
Extended soak tests and, where feasible, controlled worker interleavings
remain to be explored. Define duration, load, observed resources, and
reproduction before creating new tests; relate them to C1/C3 and QA3 scenarios.

### QA6: External compliance automation

**Status:** deferred. **Priority:** not set. **Target:** no assigned version.

**Context.** h1spec and Http11Probe are run manually. Their automation is
deferred beyond the current hardening effort. When resumed, use the versioned
adapters, pin the environment, and distinguish runner failures from protocol
failures. Preserve logs and revisions for each execution. This task
complements QA1.

## Release engineering

### RE1: Release governance and traceability

**Context.** The version comes from `include/version.h`. A license, README,
and publishing workflow gated on the entire CI matrix exist.
The existing gates are defined in the
[CI workflow](../.github/workflows/ci.yml).

**Outstanding work.** Create a changelog, security policy, and contribution
guide. Define channels for vulnerabilities, compatibility, and contributions.

**Components.** Public project documents, version, and release procedure;
modify the workflow only if the selected mechanism requires it.

**Acceptance and verification.**

- Each document publishes procedures and channels that are actually available.
- Every tag points to a revision with a passing CI matrix and synchronized
  documentation.
- The version and notes describe the published content.
- Resolve how to publish `0.1.0-beta.1`: the current workflow derives only
  `vMAJOR.MINOR.PATCH` and does not express prerelease suffixes. This workflow
  observation requires a decision; it does not imply an agreed modification.

**Dependencies.** CI and CMake consumer validations must continue to pass.
C1/C3 must pass their real-socket tests for the beta target. The development
guide does not replace a public contribution guide with channels and a
procedure.

## C++ maintainability

### DT1: platform.h dependencies and global effects

**Context.** `platform.h` centralizes standard/system headers, `INLINE`,
Windows macros, warning 4996 suppression, and linker pragmas.
On other platforms, `tcpip.h` neither selects a backend nor explicitly
diagnoses the lack of support.

**Risk.** Macros and warnings affect consumers; transitive includes hide
dependencies and produce confusing errors.

**Components.** `include/platform.h`, selectors and backends, and every
header relying on its includes.

**Acceptance and verification.** Inventory actual uses, justify each
macro/pragma, and compile public headers independently. Define a diagnostic
for unsupported platforms if that is the contract. Check that consumer
options do not change inadvertently.

**Limits.** Local changes based on evidence; no general include reorganization
or expansion of supported platforms is proposed by default.

### DT2: Indexed getter contract

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

## Public documentation

### DOC1: Transport lifecycle

**Outstanding work.** Document direct transport use outside `v11::server`.

**Context to preserve.** `stop()` is called outside its workers, and callbacks
do not change during start/stop. The contract distinguishes graceful closure
from fatal abort.

**Acceptance.** Verify and document call order, allowed threads, callbacks,
restart, and startup errors with public examples. Link from architecture
and validate each example against the corresponding tests.

### DOC2: Request views and getters

**Outstanding work.** Document string_view/header_view ownership and lifetime,
and indexed getter preconditions.

**Context to preserve.** Getters do not return owning copies; request owns
the head buffer and rebases views. A borrowed reader requires external
storage to outlive its uses.

**Acceptance.** Show valid uses, invalidation on destruction, and the
precondition chosen in DT2. Cross-check lifetime tests and link the contract
from the corresponding examples.

## Beyond the first release

F1-F5 are deferred beyond the 0.1 batches. This deferral does not include
limits, slow clients, stress testing, or baselines: those retain their
previous entries.

### F1: TLS

**Context.** The first release is intended to be deployed behind a TLS
terminator, such as a reverse proxy. Original estimate: A.

**Future scope.** Integrate TLS while preserving the protocol-transport
boundary.

**Decisions and verification.** Select the dependency and ownership model
before implementation; specify handshake, closure, and errors with equivalent
tests on supported platforms.

### F2: Compression and GZIP

**Context.** `Accept-Encoding`/`Content-Encoding` negotiation and `Vary`
handling are optional capabilities.

**Decisions.** An external compression library conflicts with the current
zero-dependency claim. Resolve that policy and the set of formats before
designing the API.

**Proposed acceptance.** Consistent negotiation and `Vary` handling, correct
framing, and tests for empty, binary, and large bodies.

### F3: Progressive streaming and SSE

**Context.** The handler finishes producing the body before handing off the
response; current draining does not constitute progressive streaming.
Original estimate: A.

**Future scope.** Represent start, fragments, and completion/error; apply
byte-based backpressure and batch small fragments.

**Proposed acceptance.** Test cancellation, slow clients, partial errors,
and ordering. Preserve the cost of the one-shot path and synchronous hot
path by comparing against QA3.

### F4: Ordered upgrade barrier

**Context.** The `101` must reach the head of the response order before
transferring the channel. Original estimate: A.

**Future scope.** Stop HTTP decoding at the correct point, deliver residual
bytes to the new codec, and transfer control.

**Proposed acceptance.** Test first with a dummy codec: previous responses,
already received bytes, closure, and errors. The contract remains generic
and does not leak HTTP semantics into IOCP or epoll.

**Dependencies.** Prerequisite for F5 (WebSockets).

### F5: WebSockets

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

## Identifier mapping

The previous column refers to the backlog before renumbering.
These identifiers are retained only to interpret older references;
current links and dependencies use the new column.
Completed items do not occupy positions in the active backlog.

| Previous | New | Item |
| --- | --- | --- |
| C1 | [C1](#c1-single-inactivity-timeout) | Single inactivity timeout |
| R2 | [C2](#c2-effective-per-request-limits) | Effective per-request limits |
| C5 | [C3](#c3-global-active-connection-limit) | Global active connection limit |
| P1 | [P1](#p1-static-file-handler) | Static file handler |
| P2 | [P2](#p2-access-logging) | Access logging |
| P3 | [P3](#p3-middleware-chain) | Middleware chain |
| P4 | [P4](#p4-form-parsing) | Form parsing |
| C2 | [P5](#p5-automatic-conditionals-and-ranges) | Automatic conditionals and ranges |
| C3 | [P6](#p6-output-trailers) | Output trailers |
| C4 | [P7](#p7-automatic-resource-options) | Automatic resource OPTIONS |
| P5 | [QA1](#qa1-exhaustive-compliance-suite) | Exhaustive compliance suite |
| QA3 | [QA2](#qa2-fuzzing) | Fuzzing |
| QA4 | [QA3](#qa3-performance-baseline) | Performance baseline |
| QA5 | [QA4](#qa4-harness-diagnostics-and-isolation) | Harness diagnostics and isolation |
| No ID | [QA5](#qa5-stress-campaigns) | Stress campaigns |
| No ID | [QA6](#qa6-external-compliance-automation) | External compliance automation |
| RE4 | [RE1](#re1-release-governance-and-traceability) | Release governance and traceability |
| DT4 | [DT1](#dt1-platformh-dependencies-and-global-effects) | platform.h dependencies and global effects |
| DT5 | [DT2](#dt2-indexed-getter-contract) | Indexed getter contract |
| No ID | [DOC1](#doc1-transport-lifecycle) | Transport lifecycle |
| No ID | [DOC2](#doc2-request-views-and-getters) | Request views and getters |
| No ID | [F1](#f1-tls) | TLS |
| No ID | [F2](#f2-compression-and-gzip) | Compression and GZIP |
| No ID | [F3](#f3-progressive-streaming-and-sse) | Progressive streaming and SSE |
| No ID | [F4](#f4-ordered-upgrade-barrier) | Ordered upgrade barrier |
| No ID | [F5](#f5-websockets) | WebSockets |
