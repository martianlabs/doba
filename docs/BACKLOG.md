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

The current `0.1.0-beta.1` target is to complete C1 and C3 and verify them
over real sockets on Windows and Linux. Release also requires preserving
CI and CMake consumer validations and completing RE1.

Recommended order:

1. C1: a single inactivity timeout.
2. C3: a global active connection limit.
3. Close RE1 and verify the complete release matrix.

C1 and C3 share generic configuration before `start()` and platform-specific
execution in IOCP and epoll. Define the common part before addressing each
localized change. Default values and the exact API shape have not yet
been selected.

C2, fuzzing, and external tool automation remain deferred. P5-P7 are neither
compliance nor release gates. DT1 and DT2 do not automatically block the
first release either. Other outstanding items have no assigned version.

**Pending defect verification.** B8-B12 require focused regressions before
0.1 publication. Their source-level evidence does not establish that every
reported runtime outcome has been reproduced. Confirmed memory-safety,
information-disclosure, or wire-protocol defects require a fix or an explicit
release decision with documented impact and mitigation. B13 concerns the
benchmark adapter, not the core release. Assess C7 resource exhaustion and
deployment mitigations before deciding whether its implementation must join
the release target.

DT3 requires an explicit response contract. P8 and F6-F7 remain optional
improvements, not new 0.1 gates. Feature parity with mature frameworks is
not a release criterion.

<a name="inventory"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-inventory-dark.svg">
    <img src="../resources/docs/backlog/h2-inventory.svg" alt="Inventory">
  </picture>
</h2>

35 outstanding entries across eight categories. Each entry retains only
remaining work or an open decision, with source and test evidence checked
against the current codebase. Verification pending means that the focused
regressions or runtime measurements described by the entry remain to be run.
Category totals count entries, not confirmed bugs.

| Category | Identifiers | Total |
| --- | --- | --- |
| Operational hardening | C1-C3, C7 | 4 |
| Bugs | B8-B13 | 6 |
| Product and convenience | P2-P8 | 7 |
| Quality and validation | QA1-QA3, QA5-QA6 | 5 |
| Release engineering | RE1 | 1 |
| C++ maintainability | DT1-DT3 | 3 |
| Public documentation | DOC1-DOC2 | 2 |
| Beyond the first release | F1-F7 | 7 |
| **Total** | | **35** |

| Item | Category | Status | Priority | Target |
| --- | --- | --- | --- | --- |
| [C1](#c1-single-inactivity-timeout) | Hardening | Pending | Beta target | 0.1.0-beta.1 |
| [C2](#c2-effective-per-request-limits) | Hardening | Deferred | High | No assigned version |
| [C3](#c3-global-active-connection-limit) | Hardening | Pending | Beta target | 0.1.0-beta.1 |
| [C7](#c7-pending-response-and-work-budget) | Hardening | Verification pending | High | Assess before 0.1 |
| [B8](#b8-response-framing-after-clear_body) | Bug | Verification pending | Medium | Verify before 0.1 |
| [B9](#b9-response-driven-connection-close) | Bug | Verification pending | Medium | Verify before 0.1 |
| [B10](#b10-internal-exception-details-in-500-responses) | Security policy | Verification pending | Medium | Verify before 0.1 |
| [B11](#b11-body-suppression-for-head-error-responses) | Bug | Verification pending | Medium | Verify before 0.1 |
| [B12](#b12-trailing-data-after-an-ip-literal-authority) | Bug | Verification pending | Medium | Verify before 0.1 |
| [B13](#b13-signed-overflow-in-the-httparena-adapter) | Benchmark bug | Verification pending | Medium | Before adapter publication |
| [P2](#p2-access-logging) | Product | Pending | Not set | No assigned version |
| [P3](#p3-middleware-chain) | Product | Pending | Not set | No assigned version |
| [P4](#p4-form-parsing) | Product | Pending | Not set | No assigned version |
| [P5](#p5-automatic-conditionals-and-ranges) | Product | Deferred | Not set | No assigned version |
| [P6](#p6-output-trailers) | Product | Deferred | Not set | No assigned version |
| [P7](#p7-automatic-resource-options) | Product | Deferred | Not set | No assigned version |
| [P8](#p8-429-response-construction) | Product | Pending | Low | No assigned version |
| [QA1](#qa1-exhaustive-compliance-suite) | QA | Pending | Not set | No assigned version |
| [QA2](#qa2-fuzzing) | QA | Deferred | High | No assigned version |
| [QA3](#qa3-performance-baseline) | QA | Pending | Medium | No assigned version |
| [QA5](#qa5-stress-campaigns) | QA | Pending | Not set | No assigned version |
| [QA6](#qa6-external-compliance-automation) | QA | Deferred | Not set | No assigned version |
| [RE1](#re1-release-governance-and-traceability) | Release | Pending | Medium | 0.1.0-beta.1 |
| [DT1](#dt1-platformh-dependencies-and-global-effects) | C++ | Pending | Medium | No assigned version |
| [DT2](#dt2-indexed-getter-contract) | C++ | Pending | Low/Medium | No assigned version |
| [DT3](#dt3-response-framing-and-capacity-contract) | C++ | Decision pending | Medium | No assigned version |
| [DOC1](#doc1-transport-lifecycle) | Documentation | Pending | Not set | No assigned version |
| [DOC2](#doc2-request-views-and-getters) | Documentation | Pending | Not set | No assigned version |
| [F1](#f1-tls) | Future | Deferred | Not set | Beyond 0.1 |
| [F2](#f2-compression-and-gzip) | Future | Deferred | Not set | Beyond 0.1 |
| [F3](#f3-progressive-streaming-and-sse) | Future | Deferred | Not set | Beyond 0.1 |
| [F4](#f4-ordered-upgrade-barrier) | Future | Deferred | Not set | Beyond 0.1 |
| [F5](#f5-websockets) | Future | Deferred | Not set | Beyond 0.1 |
| [F6](#f6-listener-and-worker-configuration) | Future | Deferred | Not set | Beyond 0.1 |
| [F7](#f7-shutdown-with-draining) | Future | Deferred | Not set | Beyond 0.1 |

<a name="operational-hardening"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-operational-hardening-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-operational-hardening-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-operational-hardening-dark.svg">
    <img src="../resources/docs/backlog/h2-operational-hardening.svg" alt="Operational hardening">
  </picture>
</h2>

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

<a name="c2-effective-per-request-limits"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c2-effective-per-request-limits-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c2-effective-per-request-limits-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c2-effective-per-request-limits-dark.svg">
    <img src="../resources/docs/backlog/h3-c2-effective-per-request-limits.svg" alt="C2: Effective per-request limits">
  </picture>
</h3>

**Remaining work.** Select defaults and expose a minimal pre-start API that
passes request policies through the standard server to its decoder. Define
and enforce the selected budget for chunked bodies, whose final size is not
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

**Dependencies and decisions.** API shape, defaults, and body accounting
remain undecided. This item is deferred and controls resources per request;
C3 separately limits active connections.

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

<a name="c7-pending-response-and-work-budget"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-dark.svg">
    <img src="../resources/docs/backlog/h3-c7-pending-response-and-work-budget.svg" alt="C7: Pending response and work budget">
  </picture>
</h3>

**Status and evidence.** Verification pending. Source inspection found no
per-connection budget at response insertion or deferred slot reservation.
A socket send buffer limit does not bound queued response bodies or work.
Resource exhaustion has not been reproduced in a stress campaign.

**Risk.** A pipelining client that does not consume responses, or an early
deferred handler that delays ordered output, may accumulate responses and
retained request state while more requests are accepted.

**Components.** `responses_`, deferred reservations and receive scheduling in
[tcpip_linux.h](../include/transport/server/tcpip_linux.h) and
[tcpip_windows.h](../include/transport/server/tcpip_windows.h).

**Scope to decide.** Measure retained work and bytes; define a bounded
admission policy and its resume or rejection behavior only if required.
Preserve ordering, cancellation and ownership across synchronous and deferred
handlers. Do not prescribe a new queue abstraction or HTTP-specific transport.

**Acceptance and tests.** Use a non-reading peer and a delayed first handler
with pipelined successors on both platforms. Measure pending entries and
retained memory, verify another connection remains serviceable, and check
resume, disconnect and stop without leaks or duplicate completion.

**Release decision.** Assess before 0.1. Record the supported deployment
envelope and verify any proxy mitigation; do not assume a proxy bounds
backend pipelining. Implementation priority depends on demonstrated exposure.
C3 bounds connections, not this queue; F3 concerns future progressive output.
Coordinate validation with QA1/QA5 without duplicating those campaigns.

<a name="bugs"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-bugs-dark.svg">
    <img src="../resources/docs/backlog/h2-bugs.svg" alt="Bugs">
  </picture>
</h2>

B8-B13 retain evidence in the current source. Focused runtime regressions
and the resulting fixes or contract decisions remain outstanding. Proposed
tests below are not recorded as executed or failing. B10 includes an
error-disclosure policy decision; B13 is limited to the benchmark adapter.

<a name="b8-response-framing-after-clear_body"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b8-response-framing-after-clear_body-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b8-response-framing-after-clear_body-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b8-response-framing-after-clear_body-dark.svg">
    <img src="../resources/docs/backlog/h3-b8-response-framing-after-clear_body.svg" alt="B8: Response framing after clear_body">
  </picture>
</h3>

**Status.** Runtime regression pending; medium severity.

**Source evidence.** In
[response.h](../include/protocol/http/v11/response.h), `clear_body()` resets
the deferred length and removes explicit framing. `apply_body_framing()`
does not restore a length when that optional value is absent. An ordinary
200 response that sets and clears its body therefore has no length or
chunked framing supplied by these paths. Verify message completion on a
persistent connection and correct the empty-body framing.

**Acceptance and tests.** Serialize `ok_200()` followed by `set_body("x")`
and `clear_body()`, then verify complete wire framing and a following response
over a persistent socket. Test repeated clearing and preserve the distinct
HEAD and body-forbidden status rules. Header-removal assertions alone do not
establish message completion.

**Components and delivery.** Response unit tests and HTTP response integration
tests; verify before 0.1. Coordinate with DT3 without requiring a redesign
of explicit framing. This case involves no manual framing override.

<a name="b9-response-driven-connection-close"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b9-response-driven-connection-close-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b9-response-driven-connection-close-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b9-response-driven-connection-close-dark.svg">
    <img src="../resources/docs/backlog/h3-b9-response-driven-connection-close.svg" alt="B9: Response-driven Connection close">
  </picture>
</h3>

**Status.** Runtime regression pending; medium severity.

**Source evidence.** [server.h](../include/protocol/http/v11/server.h)
propagates request-driven closure to response headers, but there is no return
path for a handler's `Connection: close` intent.
[serialization_result](../include/protocol/serialization.h) carries bytes and
an optional body reader; both TCP backends consult the decoder's
`result.channel` when deciding closure after a response.

**Acceptance and tests.** Send a persistent request whose handler returns
`Connection: close`; assert that the full response drains before EOF.
Include a pipelined successor and verify that it is not processed beyond
the close boundary. Cover synchronous and deferred handlers on both platforms,
and preserve ordinary keep-alive and request-driven closure.

**Delivery and limits.** Verify before 0.1. Preserve the generic transport
boundary; do not add HTTP header parsing to a transport. Select the smallest
way to convey response closure without assuming a public API extension.

<a name="b10-internal-exception-details-in-500-responses"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b10-internal-exception-details-in-500-responses-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b10-internal-exception-details-in-500-responses-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b10-internal-exception-details-in-500-responses-dark.svg">
    <img src="../resources/docs/backlog/h3-b10-internal-exception-details-in-500-responses.svg" alt="B10: Internal exception details in 500 responses">
  </picture>
</h3>

**Status.** Verification and policy decision pending; medium severity.

**Evidence.** Both TCP backends forward caught `std::exception::what()` to
the error callback. The HTTP error mapping in
[server.h](../include/protocol/http/v11/server.h) uses the supplied reason
as the 500 body. Existing
[server_response_tests.cpp](../tests/integration/protocol/http/v11/server_response_tests.cpp)
expect handler failure text in that body; changing this requires an explicit
public error contract, not silently weakening the tests.

**Risk.** Application exception messages can contain internal paths, service
details or other data that should not be returned to a remote client.

**Acceptance and tests.** Decide on a generic public 500 body and where
internal diagnostics may remain available. Throw a unique sensitive marker
from synchronous and deferred handlers; verify its absence from the wire and
the selected diagnostic behavior. Preserve legitimate non-500 rejection
semantics. Update the previous disclosure expectation only after the new
contract is approved.

**Delivery and limits.** Review before 0.1. A minimal safe default does not
depend on P3, a general error-hook API, or a new logging framework.

<a name="b11-body-suppression-for-head-error-responses"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b11-body-suppression-for-head-error-responses-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b11-body-suppression-for-head-error-responses-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b11-body-suppression-for-head-error-responses-dark.svg">
    <img src="../resources/docs/backlog/h3-b11-body-suppression-for-head-error-responses.svg" alt="B11: Body suppression for HEAD error responses">
  </picture>
</h3>

**Status.** Runtime regression pending; medium severity.

**Source evidence.** Normal dispatch in
[server.h](../include/protocol/http/v11/server.h) applies HEAD body suppression
to successful handler results. Exceptions reach a separate error callback
that receives a reason code and text, without the request method. That
callback constructs a response body and bypasses `apply_response_rules()`.
Verify the error wire output and preserve HEAD suppression through recovery.

**Acceptance and tests.** Register a HEAD handler that throws; verify the
wire contains no response body. Repeat with a deferred failure and a response
serialization failure where recovery is supported. Test ordering and the
next response or EOF on both backends. Preserve GET error bodies and valid
HEAD metadata; limit method-aware behavior to requests whose method is known.

**Components and delivery.** HTTP response rules, both TCP error paths and
response integration tests. Verify before 0.1; coordinate with B10 but retain
separate assertions for disclosure and body suppression.

<a name="b12-trailing-data-after-an-ip-literal-authority"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b12-trailing-data-after-an-ip-literal-authority-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b12-trailing-data-after-an-ip-literal-authority-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b12-trailing-data-after-an-ip-literal-authority-dark.svg">
    <img src="../resources/docs/backlog/h3-b12-trailing-data-after-an-ip-literal-authority.svg" alt="B12: Trailing data after an IP-literal authority">
  </picture>
</h3>

**Status.** Focused regression pending; medium severity.

**Source evidence.** In
[helpers.h](../include/protocol/http/common/helpers.h),
`split_authority_host_port()` returns the position immediately after `]`
for an IP-literal. The absolute-form caller validates the remaining authority
only when that position contains `:`. A different suffix has no rejection
branch, so its bytes can be ignored while parsing the host.

**Reproduction candidate.**

```text
GET http://[::1]junk/ HTTP/1.1\r\nHost: a\r\n\r\n
```

**Acceptance and tests.** Verify rejection in the absolute-form helper and
through request decoding. Cover valid bracketed hosts with and without a
port, malformed suffixes and fragmented input. Enforce the existing authority
grammar without adding unrelated semantic validation or rejecting permitted
syntax in adjacent forms.

**Delivery and scope.** Verify before 0.1. Correct authority syntax while
preserving target-authority precedence over Host.

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

**Delivery.** Resolve before publishing the affected adapter as verified.
This does not by itself block the core library release.

<a name="product-and-convenience"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-product-and-convenience-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-product-and-convenience-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-product-and-convenience-dark.svg">
    <img src="../resources/docs/backlog/h2-product-and-convenience.svg" alt="Product and convenience">
  </picture>
</h2>

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

**Delivery.** Deferred optional functionality; neither a core compliance gate
nor a release gate.

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

**Out of scope.** Optional capability with no assigned release gate.

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

**Out of scope.** Optional convenience; not a release gate.

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

**Delivery.** Optional convenience with no assigned version; not a 0.1 gate.

<a name="quality-and-validation"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-quality-and-validation-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-quality-and-validation-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-quality-and-validation-dark.svg">
    <img src="../resources/docs/backlog/h2-quality-and-validation.svg" alt="Quality and validation">
  </picture>
</h2>

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
- After C2 defines configurable limits, test zero, limit-1, limit, and limit+1,
  early rejection, and encoded/decoded chunked accounting through the server.

**Components.** Decoder and framing tests, storage tests, protocol-transport
integration, and test helpers. Coordinate pending-response measurements with
C7/QA5 rather than duplicating those campaigns.

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
release baseline with regression tolerances. Pin the Web Frameworks runner,
whose adapter currently references an inspected branch rather than a commit.

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

**Status:** pending. **Priority:** not set. **Target:** no assigned version.

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

**Status:** deferred. **Priority:** not set. **Target:** no assigned version.

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

**Outstanding work.** Create a changelog, security policy, and contribution
guide. Define channels for vulnerabilities, compatibility, and contributions.
Resolve how to publish `0.1.0-beta.1`: the
[publishing workflow](../.github/workflows/ci.yml) currently derives only
`vMAJOR.MINOR.PATCH` from `include/version.h`, without a prerelease suffix.

**Components.** Public project documents, version, and release procedure;
modify the workflow only if the selected mechanism requires it.

**Acceptance and verification.**

- Publish procedures and channels that are actually available.
- Point the release tag to the exact revision with a passing CI matrix and
  synchronized documentation, including CMake consumer validation.
- Make the version and release notes describe the published content.
- Verify C1/C3 over real sockets and resolve the defect and C7 release
  decisions listed in the release target before publication.

**Dependencies.** The development guide does not replace a public
contribution guide with channels and a procedure. The prerelease mechanism
remains a decision, not an agreed workflow modification.

<a name="c-maintainability"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-c-maintainability-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-c-maintainability-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-c-maintainability-dark.svg">
    <img src="../resources/docs/backlog/h2-c-maintainability.svg" alt="C++ maintainability">
  </picture>
</h2>

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
operation order, framing ownership, and failure behavior. B8 separately covers
empty-body framing without a manual override.

**Remaining coverage.** Add shorter/longer explicit-length cases and manual
Transfer-Encoding cases according to the selected contract, checking complete
wire output. Preserve the
[existing tests](../tests/unit/protocol/http/v11/response_tests.cpp) for matching
lengths, conflicting Content-Length/Transfer-Encoding, HEAD/bodyless responses,
and exact capacity with a fixed Date and deferred Content-Length.

**Capacity follow-up.** Document the fixed status/header budget and the space
needed for generated fields. Add exact-fit and over-capacity cases combining
automatic Date with deferred framing, and verify recovery cannot publish a
partial malformed response. Coordinate method-aware error recovery with B11;
keep header capacity distinct from body storage and progressive streaming.

**Delivery and limits.** Publish the selected response preconditions. No
automatic release gate, dynamic buffers, ownership redesign, broad serializer
refactor, or public API change is prescribed.

<a name="public-documentation"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-public-documentation-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-public-documentation-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-public-documentation-dark.svg">
    <img src="../resources/docs/backlog/h2-public-documentation.svg" alt="Public documentation">
  </picture>
</h2>

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

F1-F7 are deferred beyond the 0.1 batches. This deferral does not include
limits, slow clients, stress testing, or baselines: those retain their
previous entries.

<a name="f1-tls"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f1-tls-dark.svg">
    <img src="../resources/docs/backlog/h3-f1-tls.svg" alt="F1: TLS">
  </picture>
</h3>

**Context.** The first release is intended to be deployed behind a TLS
terminator, such as a reverse proxy. Original estimate: A.

**Future scope.** Integrate TLS while preserving the protocol-transport
boundary.

**Decisions and verification.** Select the dependency and ownership model
before implementation; specify handshake, closure, and errors with equivalent
tests on supported platforms.

<a name="f2-compression-and-gzip"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-f2-compression-and-gzip-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f2-compression-and-gzip-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f2-compression-and-gzip-dark.svg">
    <img src="../resources/docs/backlog/h3-f2-compression-and-gzip.svg" alt="F2: Compression and GZIP">
  </picture>
</h3>

**Context.** `Accept-Encoding`/`Content-Encoding` negotiation and `Vary`
handling are optional capabilities.

**Decisions.** An external compression library conflicts with the current
zero-dependency claim. Resolve that policy and the set of formats before
designing the API.

**Proposed acceptance.** Consistent negotiation and `Vary` handling, correct
framing, and tests for empty, binary, and large bodies.

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
pre-start bind-address and worker-count options, following the configuration
contract selected for C1/C3. Do not introduce a general networking abstraction
or expand supported address families by default.

**Components and acceptance.** Server configuration and both TCP backends.
Test loopback-only binding, invalid or unavailable addresses, default behavior,
worker validation, startup failure cleanup and stop on both platforms.

**Delivery.** Beyond 0.1; record current deployment constraints without
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

**Delivery.** Beyond 0.1, not an implied release blocker. Until implemented,
document abrupt-stop limitations and verify any external deployment-draining
procedure before relying on it. Coordinate public lifecycle documentation
with DOC1; do not count documentation alone as implementation.
