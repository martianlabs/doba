<a name="backlog"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h1-backlog-dark.svg">
    <img src="../resources/docs/backlog/h1-backlog.svg" alt="Backlog">
  </picture>
</h1>

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
- [Identifier mapping](#identifier-mapping)

<a name="release-target"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-release-target-dark.svg">
    <img src="../resources/docs/backlog/h2-release-target.svg" alt="Release target">
  </picture>
</h2>

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

**Delivery debt from the 2026-09-15 review.** B7-B12 require verification
before 0.1 publication, not an assumption that every reported mechanism has
already been reproduced. Confirmed memory-safety, information-disclosure,
or wire-protocol defects require a fix or an explicit release decision with
documented impact and mitigation. B13 concerns the benchmark adapter, not
the core release. Assess C7 resource exhaustion and deployment mitigations
before deciding whether its implementation must join the release target.

DT1 has a localized header-compilation follow-up; it does not require a
general include refactor. DT3 requires an explicit response contract, not a
buffer redesign. P8 and F6-F7 are optional improvements, not new 0.1 gates.
Existing deferrals remain in force. Feature parity with mature frameworks
is not a release criterion; ORM, sessions, and integrated JSON are not added
as requirements by this review.

<a name="inventory"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-inventory-dark.svg">
    <img src="../resources/docs/backlog/h2-inventory.svg" alt="Inventory">
  </picture>
</h2>

47 entries across eight categories. C4-C6, B1-B6 and QA4 are completed and
retained for traceability. Numbering identifies items; it does not express
priority or implementation order.

C4-C6 and B1-B6 tracked ten findings behind 15 failing unit tests. The
original classification remains seven production bugs, one test bug and two
contract decisions. C5 separates query truncation from full-buffer handling;
B6 records a compatibility extension, not a previously guaranteed behavior.
Category totals count entries, not bugs.

| Original classification | Findings | Originally failing unit tests |
| --- | --- | --- |
| Confirmed production bugs | B1-B3, B5, C4, C5 query overflow, C6 | 11 |
| Confirmed test bug | B4 | 1 |
| Contract decisions | C5 full buffer, B6 | 3 |

The 2026-09-13 revalidation recorded 202 passing and 15 failing cases across
the eight affected suites on Windows/MSVC and Linux/GCC. Rebuilding and
running without exclusions on 2026-09-14 reproduced the same baseline.
The corrected suites now pass all 222 cases on both platforms. Additional
router and real-socket tests cover handler registration, authority getters,
query overflow and fragmented head capacity on IOCP and epoll.

Final local validation passed with strict warnings: MSVC, GCC and Clang in
Debug and Release; Clang ASan with leak detection, UBSan and TSan; and
CMake 3.20.6. Each configuration passed the complete unit/integration suites
and both harness checks. Windows runs 816 unit cases, Linux runs 815, and
both run 95 integration cases. Installed Debug/Release consumers and the
`add_subdirectory` consumer also built and ran successfully.

**Temporary CI bypass removed (2026-09-14).** All 15 regressions are active,
including the corrected negative B4 case and the approved C5/B6 contracts.
The temporary `DOBA_SKIP_BACKLOG_UNIT_TESTS` option, its exclusion list and
its CI activation have been removed. The runner's general `--exclude`
support and its harness checks remain available.

| Category | Identifiers | Total |
| --- | --- | --- |
| Operational hardening | C1-C7 | 7 |
| Bugs | B1-B13 | 13 |
| Product and convenience | P1-P8 | 8 |
| Quality and validation | QA1-QA6 | 6 |
| Release engineering | RE1 | 1 |
| C++ maintainability | DT1-DT3 | 3 |
| Public documentation | DOC1-DOC2 | 2 |
| Beyond the first release | F1-F7 | 7 |
| **Total** | | **47** |

| Item | Category | Status | Priority | Target |
| --- | --- | --- | --- | --- |
| [C1](#c1-single-inactivity-timeout) | Hardening | Pending | Beta target | 0.1.0-beta.1 |
| [C2](#c2-effective-per-request-limits) | Hardening | Deferred | High | No assigned version |
| [C3](#c3-global-active-connection-limit) | Hardening | Pending | Beta target | 0.1.0-beta.1 |
| [C4](#c4-absolute-form-authority-precedence) | Hardening | Completed | Not set | No assigned version |
| [C5](#c5-internal-decoder-capacity-overflow) | Hardening | Completed | Not set | No assigned version |
| [C6](#c6-te-connection-option) | Hardening | Completed | Not set | No assigned version |
| [C7](#c7-pending-response-and-work-budget) | Hardening | Verification pending | High | Assess before 0.1 |
| [B1](#b1-entity-tag-backslash-rejection) | Bug | Completed | Not set | No assigned version |
| [B2](#b2-via-escaped-comment-rejection) | Bug | Completed | Not set | No assigned version |
| [B3](#b3-via-comment-comma-splitting) | Bug | Completed | Not set | No assigned version |
| [B4](#b4-incorrect-ipv6-expectation-in-via-test) | Bug | Completed | Not set | No assigned version |
| [B5](#b5-duplicate-automatic-date-after-header-removal) | Bug | Completed | Not set | No assigned version |
| [B6](#b6-noexcept-handler-signature-rejection) | Compatibility | Completed | Not set | No assigned version |
| [B7](#b7-empty-body-in-the-expect-continue-example) | Example bug | Verification pending | High | Verify before 0.1 |
| [B8](#b8-response-framing-after-clear_body) | Bug | Verification pending | Medium | Verify before 0.1 |
| [B9](#b9-response-driven-connection-close) | Bug | Verification pending | Medium | Verify before 0.1 |
| [B10](#b10-internal-exception-details-in-500-responses) | Security policy | Verification pending | Medium | Verify before 0.1 |
| [B11](#b11-body-suppression-for-head-error-responses) | Bug | Verification pending | Medium | Verify before 0.1 |
| [B12](#b12-trailing-data-after-an-ip-literal-authority) | Bug | Verification pending | Medium | Verify before 0.1 |
| [B13](#b13-signed-overflow-in-the-httparena-adapter) | Benchmark bug | Verification pending | Medium | Before adapter publication |
| [P1](#p1-static-file-handler) | Product | Pending | Not set | No assigned version |
| [P2](#p2-access-logging) | Product | Pending | Not set | No assigned version |
| [P3](#p3-middleware-chain) | Product | Pending | Not set | No assigned version |
| [P4](#p4-form-parsing) | Product | Pending | Not set | No assigned version |
| [P5](#p5-automatic-conditionals-and-ranges) | Product | Deferred | Not set | No assigned version |
| [P6](#p6-output-trailers) | Product | Deferred | Not set | No assigned version |
| [P7](#p7-automatic-resource-options) | Product | Deferred | Not set | No assigned version |
| [P8](#p8-429-response-construction) | Product | Pending | Low | No assigned version |
| [QA1](#qa1-exhaustive-compliance-suite) | QA | Partial | Not set | No assigned version |
| [QA2](#qa2-fuzzing) | QA | Deferred | High | No assigned version |
| [QA3](#qa3-performance-baseline) | QA | Pending | Medium | No assigned version |
| [QA4](#qa4-harness-diagnostics-and-isolation) | QA | Completed | Medium | No assigned version |
| [QA5](#qa5-stress-campaigns) | QA | Pending | Not set | No assigned version |
| [QA6](#qa6-external-compliance-automation) | QA | Deferred | Not set | No assigned version |
| [RE1](#re1-release-governance-and-traceability) | Release | Partial | Medium | 0.1.0-beta.1 |
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

<a name="c4-absolute-form-authority-precedence"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c4-absolute-form-authority-precedence-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c4-absolute-form-authority-precedence-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c4-absolute-form-authority-precedence-dark.svg">
    <img src="../resources/docs/backlog/h3-c4-absolute-form-authority-precedence.svg" alt="C4: Absolute-form authority precedence">
  </picture>
</h3>

**Status.** Completed 2026-09-14.

**Resolution.** Absolute-form no longer requires equality with Host. The target
authority is effective; `get_host()` and its port/type getters retain the
received Host, while `get_target_authority_host()` and its port/type getters
expose the target authority. The raw header, Host validation and CONNECT
behavior are preserved. Unit and real-socket tests verify differing hosts,
ports, default-port cases and getter values on both platforms.

**Historical finding and acceptance criteria.**

**Type.** Confirmed bug (HTTP processing).

**Context.** GET http://a/ with Host: b is rejected. The routing rule
requires equality between Host and the request-target authority, including
absolute-form. This rejection was reproduced against the current decoder.

**Scope.** Correct absolute-form origin-server authority processing to use
the request-target authority. Preserve Host syntax/multiplicity validation,
the raw header value and public signatures. Keep CONNECT/authority-form
behavior separate.

**Components.** [routing.h](../include/protocol/http/v11/headers/rules/routing.h),
[decoder.h](../include/protocol/http/v11/decoder.h), routing unit tests,
decoder tests and HTTP socket integration tests.

**Existing failing unit tests.**

- [routing_tests.cpp](../tests/unit/protocol/http/v11/headers/rules/routing_tests.cpp):
  `absolute form accepts differing Host ports`.
- [routing_tests.cpp](../tests/unit/protocol/http/v11/headers/rules/routing_tests.cpp):
  `absolute form authority overrides a different Host name`.
- [decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp):
  `absolute form uses its authority when Host differs`.

**Acceptance and tests.**

- Accept valid absolute-form requests whose Host differs from the target.
- Check differing hosts/ports and default-port equivalence.
- Verify effective authority and preservation of the received Host value.
- Preserve rejection of missing, duplicate or syntactically invalid Host.
- Run focused regressions and equivalent HTTP cases in IOCP and epoll.

**Dependencies and decisions.** Define the existing get_host versus
get_target_authority_host contract before implementation. A contract change
must be explicit; adding these regressions does not itself correct routing.

**Reference.** [RFC 9112 S3.2.2](https://www.rfc-editor.org/rfc/rfc9112.html#section-3.2.2).

<a name="c5-internal-decoder-capacity-overflow"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c5-internal-decoder-capacity-overflow-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c5-internal-decoder-capacity-overflow-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c5-internal-decoder-capacity-overflow-dark.svg">
    <img src="../resources/docs/backlog/h3-c5-internal-decoder-capacity-overflow.svg" alt="C5: Internal decoder capacity overflow">
  </picture>
</h3>

**Status.** Completed 2026-09-14.

**Resolution.** The decoder collects up to 129 query pairs and rejects an excess
before mounting a request or initializing body storage. Empty pairs do not
count. The bounded-output helper remains unchanged.

The approved full-buffer contract rejects an incomplete head immediately
at capacity. Both capacity failures return `kInvalidSource` with the existing
generic reason, yielding 400 Bad Request. A complete 5120-byte head remains
accepted; raw and chunked bodies can span buffers. Unit and socket tests cover
127/128/129 pairs, empty separators, 5119/5120/5121-byte heads, fragmented
delivery with the excess byte withheld, connection closure and no dispatch
on rejection. The transport backends required no changes.

**Historical finding and acceptance criteria.**

**Type.** Confirmed query overflow bug; full-buffer contract pending.

**Query overflow - confirmed bug.** A request with 129 query pairs succeeds
with only 128 visible pairs, while limits.h documents rejection above 128.
The decoder exposes a truncated query without reporting the overflow.

**Cause and scope.** `split_query_parameters` explicitly documents dropping
pairs beyond the output capacity and meets that helper contract.
`mount_request_getter` does not detect the excess before mounting the request.
Reject query overflow as documented in limits.h; do not classify the helper
as independently defective. Preserve accepted queries and public signatures.

**Components.** [decoder.h](../include/protocol/http/v11/decoder.h),
[limits.h](../include/protocol/http/v11/limits.h) and
[helpers.h](../include/protocol/http/common/helpers.h), with their unit tests.

**Existing failing unit test.**

- [decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp):
  `rejects query parameter overflow without truncating`.

**Acceptance and tests.** Check 127/128/129 query pairs, including empty pair
separators. Verify that overflow returns a rejection without a partial request
and that accepted boundary inputs retain every pair.

**Full buffer - contract pending.** A complete 5121-byte head fills the
5120-byte buffer. Deserialization reports `kMoreBytesNeeded`; a subsequent
accumulation consumes zero and deserialization reports the same status.
This reproduces decoder non-progress, not a permanent server hang.

Both [Linux](../include/transport/server/tcpip_linux.h) and
[Windows](../include/transport/server/tcpip_windows.h) detect zero consumption
when more received bytes remain to be delivered and enter their error path.
This protection was verified by code inspection, not a new socket test.
If no further bytes arrive, inactivity handling remains the separate C1 issue.

**Scope to define.** Decide whether the decoder must reject an incomplete
head immediately at capacity or whether the existing transport detection
satisfies the contract. No explicit immediate-rejection requirement was found
in the reviewed decoder contract. Do not count the current test expectation
as proof of a server stability or memory-safety defect.

**Components.** [decoder.h](../include/protocol/http/v11/decoder.h),
[limits.h](../include/protocol/http/v11/limits.h), both transport backends,
decoder unit tests and HTTP socket integration tests.

**Existing failing unit test.**

- [decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp):
  `a full incomplete head terminates instead of stalling`.

**Proposed acceptance and tests.** After selecting the contract, check complete
heads of 5119/5120/5121 bytes and fragmented delivery. Distinguish the decoder
result from transport rejection. Verify that oversized requests are not
dispatched and that both backends terminate safely when excess bytes arrive.

**Dependencies and decisions.** Select the query overflow error/status and
the full-buffer responsibility before implementing the respective changes.
The full-buffer test currently assumes immediate `kInvalidSource`; its
expectation must be justified against the selected contract. These are
internal capacity policies, not RFC limits, and no automatic 431 policy is
assumed. Keep them separate from the configurable resource-limit API in C2.

<a name="c6-te-connection-option"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c6-te-connection-option-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c6-te-connection-option-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c6-te-connection-option-dark.svg">
    <img src="../resources/docs/backlog/h3-c6-te-connection-option.svg" alt="C6: TE connection option">
  </picture>
</h3>

**Status.** Completed 2026-09-14.

**Resolution.** The specific `te` prohibition and its rule comment were removed.
The existing TE dispatcher required no change. Tests accept `TE: trailers`
with `Connection: TE` and reject `gzip;q=1.001` through value validation,
covering field-name case and OWS variants.

**Historical finding and acceptance criteria.**

**Type.** Confirmed bug (HTTP processing).

**Context.** TE: trailers together with Connection: TE is rejected because
the directives rule forbids the te connection option. This rejection was
reproduced against the current decoder. A TE sender must include that option.

**Scope.** Remove the specific inappropriate te prohibition, update its
rule documentation and preserve all unrelated directive validation.

**Components.** [directives.h](../include/protocol/http/v11/headers/rules/directives.h),
[decoder.h](../include/protocol/http/v11/decoder.h), their unit tests.

**Existing failing unit tests.**

- [directives_tests.cpp](../tests/unit/protocol/http/v11/headers/rules/directives_tests.cpp):
  `accepts the TE connection option`.
- [decoder_tests.cpp](../tests/unit/protocol/http/v11/decoder_tests.cpp):
  `accepts TE with its required connection option`.

**Acceptance and tests.**

- Accept TE: trailers with Connection: TE.
- Reject TE: gzip;q=1.001 in the same context.
- Exercise canonical/lower/upper field names with and without OWS.
- Prove that the invalid value reaches its field validator; rejection of
  the request context must not mask a missing TE value check.
- Preserve existing unrelated connection-directive tests.

**Dependencies and decisions.** Correct the directive rule so that the
positive and negative cases can exercise TE value validation independently.
No public API or protocol-upgrade feature is required.

**Reference.** [RFC 9110 S10.1.4](https://www.rfc-editor.org/rfc/rfc9110.html#section-10.1.4).

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

B1-B5 are completed fixes; B4 corrected a unit-test expectation. B6 is a
completed compatibility extension under an explicitly approved contract.
C4, C5 query overflow and C6 retain their hardening identifiers. C5 full-buffer
handling is a separate approved contract. The historical findings below retain
their original classification and acceptance criteria for traceability.

B7-B13 are delivery-debt findings from source review, with runtime
verification still pending. Their proposed tests are not reported as executed
or failing. B10 includes an error-disclosure policy decision; B13 is limited
to the benchmark adapter. Keep these distinct from the completed fixes above.

<a name="b1-entity-tag-backslash-rejection"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b1-entity-tag-backslash-rejection-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b1-entity-tag-backslash-rejection-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b1-entity-tag-backslash-rejection-dark.svg">
    <img src="../resources/docs/backlog/h3-b1-entity-tag-backslash-rejection.svg" alt="B1: Entity-tag backslash rejection">
  </picture>
</h3>

**Status.** Completed 2026-09-14.

**Resolution.** If-Match and If-None-Match now delimit opaque entity-tags locally
and validate each member with `is_entity_tag`. Backslash remains literal.
Both 256-byte matrices and the expanded list boundaries pass. The shared list
helper, weak-tag syntax, wildcard and recipient OWS/empty-element contracts
are unchanged.

**Historical finding and acceptance criteria.**

**Type.** Confirmed bug (conditional header syntax).

**Context.** Both If-Match and If-None-Match reject a strong or weak entity-tag
whose opaque value is the single byte 0x5c. The byte-matrix tests reproduce
this with DQUOTE, 0x5c, DQUOTE, optionally prefixed by W/.

**Cause.** The header checks use `for_each_list_element`, which treats the
backslash as a quoted-string escape and skips the closing quote.

**Scope.** Accept the literal byte as `etagc` without changing it. Preserve
the syntax/semantic distinction for weak tags and existing list handling.

**Components.** [helpers.h](../include/protocol/http/common/helpers.h),
[if_match.h](../include/protocol/http/common/headers/if_match.h),
[if_none_match.h](../include/protocol/http/common/headers/if_none_match.h).

**Existing failing unit tests.**

- [if_match_tests.cpp](../tests/unit/protocol/http/common/headers/if_match_tests.cpp):
  `check applies the entity tag byte grammar`.
- [if_none_match_tests.cpp](../tests/unit/protocol/http/common/headers/if_none_match_tests.cpp):
  `check applies the entity tag byte grammar`.

**Acceptance and tests.** Make both byte-matrix regressions pass; retain the
valid/invalid octet boundaries, tag lists, and wildcard cases.

**Dependencies and decisions.** B2 and B3 share the list helper but have
different grammars. Preserve quoted-string handling for other consumers.

**Reference.** [RFC 9110 S8.8.3](https://www.rfc-editor.org/rfc/rfc9110.html#section-8.8.3).

<a name="b2-via-escaped-comment-rejection"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b2-via-escaped-comment-rejection-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b2-via-escaped-comment-rejection-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b2-via-escaped-comment-rejection-dark.svg">
    <img src="../resources/docs/backlog/h3-b2-via-escaped-comment-rejection.svg" alt="B2: Via escaped comment rejection">
  </picture>
</h3>

**Status.** Completed 2026-09-14.

**Resolution.** Via now delimits its list locally, consuming complete comments
with the existing `consume_comment` helper. Escapes, nested comments and
literal quotes retain their bytes. The expanded valid/invalid comment
regressions pass; shared helpers are unchanged.

**Historical finding and acceptance criteria.**

**Type.** Confirmed bug (Via comment syntax).

**Context.** `Via: 1.1 proxy (a\)b\(c)` is rejected by `via::check`.
The comment contains escaped parentheses.

**Cause.** `for_each_list_element` rejects a backslash outside a quoted
string before Via can validate the comment.

**Scope.** Accept valid quoted-pairs in comments and preserve the parsed
comment bytes.

**Components.** [via.h](../include/protocol/http/v11/headers/via.h),
[helpers.h](../include/protocol/http/common/helpers.h).

**Existing failing unit tests.**

- [via_tests.cpp](../tests/unit/protocol/http/v11/headers/via_tests.cpp):
  `check accepts protocol and comment boundaries`.

**Acceptance and tests.** Make the escaped-parentheses case pass; preserve
nested-comment acceptance and rejection of incomplete escapes, unterminated
comments, and trailing junk.

**Dependencies and decisions.** Coordinate with B3 at the shared list
boundary; do not broaden unrelated header grammars.

**References.** [RFC 9110 S5.6.5](https://www.rfc-editor.org/rfc/rfc9110.html#section-5.6.5)
and [S7.6.3](https://www.rfc-editor.org/rfc/rfc9110.html#section-7.6.3).

<a name="b3-via-comment-comma-splitting"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b3-via-comment-comma-splitting-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b3-via-comment-comma-splitting-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b3-via-comment-comma-splitting-dark.svg">
    <img src="../resources/docs/backlog/h3-b3-via-comment-comma-splitting.svg" alt="B3: Via comment comma splitting">
  </picture>
</h3>

**Status.** Completed 2026-09-14.

**Resolution.** The Via scan consumes each comment before considering separators.
Tests verify two members for the original input, nested commas, escaped
parentheses, empty list elements and OWS. This shares the localized B2 fix
and preserves the other list consumers.

**Historical finding and acceptance criteria.**

**Type.** Confirmed bug (Via list separation).

**Context.** `Via: 1.1 proxy (a,b), 1.0 other` is rejected.

**Cause.** The list helper tracks quoted strings but not comment nesting,
so it interprets the comma inside the comment as a member separator.

**Scope.** Parse two members, preserving `(a,b)` as the first comment and
`other` as the second received-by value.

**Components.** [via.h](../include/protocol/http/v11/headers/via.h),
[helpers.h](../include/protocol/http/common/helpers.h).

**Existing failing unit tests.**

- [via_tests.cpp](../tests/unit/protocol/http/v11/headers/via_tests.cpp):
  `check keeps commas inside comments in one member`.

**Acceptance and tests.** Make the two-member regression pass. Keep commas
inside nested comments and preserve separation outside comments, whitespace
handling, and invalid-comment rejection.

**Dependencies and decisions.** Coordinate with B2; retain B1's distinct
entity-tag grammar and the behavior of other list consumers.

**References.** [RFC 9110 S5.6.5](https://www.rfc-editor.org/rfc/rfc9110.html#section-5.6.5)
and [S7.6.3](https://www.rfc-editor.org/rfc/rfc9110.html#section-7.6.3).

<a name="b4-incorrect-ipv6-expectation-in-via-test"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b4-incorrect-ipv6-expectation-in-via-test-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b4-incorrect-ipv6-expectation-in-via-test-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b4-incorrect-ipv6-expectation-in-via-test-dark.svg">
    <img src="../resources/docs/backlog/h3-b4-incorrect-ipv6-expectation-in-via-test.svg" alt="B4: Incorrect IPv6 expectation in Via test">
  </picture>
</h3>

**Status.** Completed 2026-09-14.

**Resolution.** The exact IPv6 input is now a negative regression named
`check rejects an IPv6 received by host`. Its expectation and RFC comment
were corrected. No production parser change was made for B4; valid
pseudonyms and optional ports retain their coverage.

**Historical finding and acceptance criteria.**

**Type.** Confirmed bug (unit-test expectation and RFC comment).

**Context.** The unit test expects `via::check("HTTP/1.1 [::1]:8080", parsed)`
to succeed. It returns false. The test's comment incorrectly claims that
RFC 9110 S7.6.3 allows `uri-host` in received-by.

**Cause.** The expectation uses an obsolete grammar. RFC 9110 defines
received-by using a token pseudonym and optional port; its appendix B.2
records the removal of `uri-host`. Bracketed IPv6 does not match that grammar.

**Scope.** Correct the expectation and RFC comment to verify rejection under
the current contract. Rename the case to describe its negative expectation.
Do not change the production parser to satisfy the erroneous positive test.

**Components.** [via_tests.cpp](../tests/unit/protocol/http/v11/headers/via_tests.cpp).
Compare against [via.h](../include/protocol/http/v11/headers/via.h).

**Existing failing unit tests.**

- [via_tests.cpp](../tests/unit/protocol/http/v11/headers/via_tests.cpp):
  `check accepts an IPv6 received by host`.

**Acceptance and tests.** Retain the exact bracketed IPv6 input as a negative
case. Preserve valid pseudonyms with optional ports and the separate comment
regressions tracked by B2/B3. Correcting this test must not hide those bugs.

**References.** [RFC 9110 S7.6.3](https://www.rfc-editor.org/rfc/rfc9110.html#section-7.6.3)
and [appendix B.2](https://www.rfc-editor.org/rfc/rfc9110.html#appendix-B.2).

<a name="b5-duplicate-automatic-date-after-header-removal"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b5-duplicate-automatic-date-after-header-removal-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b5-duplicate-automatic-date-after-header-removal-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b5-duplicate-automatic-date-after-header-removal-dark.svg">
    <img src="../resources/docs/backlog/h3-b5-duplicate-automatic-date-after-header-removal.svg" alt="B5: Duplicate automatic Date after header removal">
  </picture>
</h3>

**Status.** Completed 2026-09-14.

**Resolution.** After removing Date, the marker is recalculated from the remaining
headers, matching the existing Content-Length/Transfer-Encoding pattern.
The original serialized-prefix regression and a new last-Date removal test
pass, covering case-insensitive names, automatic generation and other headers.
The add/remove/serialize API and ownership contract are unchanged.

**Historical finding and acceptance criteria.**

**Type.** Confirmed bug (response header mutation).

**Context.** Add Date fields with values `first` and `remaining`, remove
`date` once, then serialize. The remaining explicit field is still present,
but serialization appends an additional automatic Date.

**Cause.** `remove_header` clears `has_date_header_` without checking whether
another Date field remains. `serialize` then adds the automatic field.

**Scope.** Preserve the remaining explicit Date and prevent an extra automatic
field. The marker values reproduce the generic header-mutation contract;
this case does not request acceptance of duplicate Date fields on the wire.

**Components.** [response.h](../include/protocol/http/v11/response.h).

**Existing failing unit tests.**

- [response_tests.cpp](../tests/unit/protocol/http/v11/response_tests.cpp):
  `removing one Date preserves the remaining explicit Date`.

**Acceptance and tests.** Make the exact serialized-prefix regression pass.
Check that removing the last explicit Date still permits automatic Date
generation, and preserve case-insensitive name handling and other headers.

**Dependencies and decisions.** Keep the existing add/remove/serialize API
and mutation semantics; changing header validation is a separate decision.

<a name="b6-noexcept-handler-signature-rejection"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b6-noexcept-handler-signature-rejection-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b6-noexcept-handler-signature-rejection-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b6-noexcept-handler-signature-rejection-dark.svg">
    <img src="../resources/docs/backlog/h3-b6-noexcept-handler-signature-rejection.svg" alt="B6: Noexcept handler signature rejection">
  </picture>
</h3>

**Status.** Completed 2026-09-14.

**Resolution.** The approved contract supports const and mutable `noexcept`
call operators for the existing sync and async shapes. Four specializations
reuse the current signature bases. Tests cover qualifiers, routing parameter
counts and actual registration/execution. Compilation probes also verify
rejection of incompatible request, cancellation and return types with and
without `noexcept`. The router implementation required no changes.

**Historical finding and acceptance criteria.**

**Type.** Confirmed compatibility limitation; support contract pending.

**Context.** A synchronous lambda taking `const request&` and returning
`response` is rejected by `router_handler_lambda` when declared `noexcept`.
The asynchronous equivalent taking `shared_ptr<const request>` and
`stop_token`, returning `task<response>`, is likewise rejected by
`router_async_handler_lambda`. The constraints on `router::add` prevent
registration of these handlers.

**Cause.** The member-function-pointer specializations cover const and
non-const call operators, but omit their noexcept-qualified counterparts.

**Contract evidence.** The [architecture](ARCHITECTURE.md) describes handler
argument and return shapes without explicitly guaranteeing noexcept support.
The tests demonstrate the limitation; their positive expectations do not
independently establish a previously required compatibility guarantee.

**Scope to define.** Decide whether those otherwise supported handler shapes
must also work with noexcept. If support is required, recognize the selected
qualifier combinations while preserving argument/return checks and additional
parameter counts. Otherwise, document the restriction and justify the test
expectations against that contract.

**Components.** [router_handler_signature.h](../include/protocol/http/common/router_handler_signature.h),
[router.h](../include/protocol/http/common/router.h), handler documentation
and signature unit tests.

**Existing failing unit tests.**

- [router_handler_signature_tests.cpp](../tests/unit/protocol/http/common/router_handler_signature_tests.cpp):
  `noexcept sync handlers retain signature compatibility`.
- [router_handler_signature_tests.cpp](../tests/unit/protocol/http/common/router_handler_signature_tests.cpp):
  `noexcept async handlers retain signature compatibility`.

**Proposed acceptance and tests.** Record the supported qualifiers, then align
implementation and focused tests with that decision. Preserve existing
ordinary and mutable handlers, parameter counts, and rejection of unsupported
request/return signatures.

**Dependencies and decisions.** Keep this finding pending without counting it
as a confirmed production bug until the support contract is established.
It is not a protocol violation or a demonstrated memory-safety defect.

<a name="b7-empty-body-in-the-expect-continue-example"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b7-empty-body-in-the-expect-continue-example-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b7-empty-body-in-the-expect-continue-example-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b7-empty-body-in-the-expect-continue-example-dark.svg">
    <img src="../resources/docs/backlog/h3-b7-empty-body-in-the-expect-continue-example.svg" alt="B7: Empty body in the expect-continue example">
  </picture>
</h3>

**Status.** Verification pending; high impact within the shipped example.

**Evidence and hypothesis.** The `/echo` handler in
[expect_continue/main.cpp](../examples/http/v11/expect_continue/main.cpp)
calls `get_body_reader()->read()` without checking reader availability.
A POST with no body framing or `Content-Length: 0` may have no reader.
Dereferencing it would crash the example; this is not evidence that every
application handler has the same defect.

**Acceptance and tests.** Reproduce both empty-body requests, including the
applicable Expect path; define empty echo or explicit rejection and verify
no null dereference. Preserve non-empty and chunked echo behavior. Prefer
a local reader check consistent with existing examples; no API change.

**Delivery.** Verify and correct before 0.1 if confirmed. Keep the regression
focused on the example's handler and request-body contract.

<a name="b8-response-framing-after-clear_body"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b8-response-framing-after-clear_body-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b8-response-framing-after-clear_body-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b8-response-framing-after-clear_body-dark.svg">
    <img src="../resources/docs/backlog/h3-b8-response-framing-after-clear_body.svg" alt="B8: Response framing after clear_body">
  </picture>
</h3>

**Status.** Verification pending; medium severity.

**Evidence and hypothesis.** In
[response.h](../include/protocol/http/v11/response.h), `clear_body()` resets
the deferred length and removes explicit framing. `apply_body_framing()`
does not restore a length when that optional value is absent. A 200 response
that sets a body and then clears it may therefore have neither body framing
nor connection closure, leaving a persistent client waiting for completion.

**Acceptance and tests.** Serialize `ok_200()` followed by `set_body("x")`
and `clear_body()`, then verify complete wire framing and a following response
over a persistent socket. Test repeated clearing and preserve the distinct
HEAD and body-forbidden status rules. Existing checks for header removal
alone do not establish correct message completion.

**Components and delivery.** Response unit tests and HTTP response integration
tests; verify before 0.1. Coordinate with DT3 without requiring a redesign
of explicit framing. A confirmed ordinary-response framing defect is not
an optional feature gap.

<a name="b9-response-driven-connection-close"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b9-response-driven-connection-close-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b9-response-driven-connection-close-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b9-response-driven-connection-close-dark.svg">
    <img src="../resources/docs/backlog/h3-b9-response-driven-connection-close.svg" alt="B9: Response-driven Connection close">
  </picture>
</h3>

**Status.** Verification pending; medium severity.

**Evidence and hypothesis.** A response can advertise `Connection: close`,
but transport closure is driven by request-side channel intent. Inspect
[server.h](../include/protocol/http/v11/server.h),
[serialization.h](../include/protocol/serialization.h), and both TCP backends
to verify whether response intent reaches the connection lifecycle.

**Acceptance and tests.** Send a persistent request whose handler returns
`Connection: close`; assert that the full response drains before EOF.
Include a pipelined successor and verify that it is not processed beyond
the close boundary. Cover synchronous and deferred handlers on both platforms,
and preserve ordinary keep-alive and request-driven closure.

**Delivery and limits.** Verify before 0.1. Preserve the generic transport
boundary; do not add HTTP header parsing to a transport or assume a public
API extension is necessary before tracing the complete path.

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

**Status.** Verification pending; medium severity.

**Evidence and hypothesis.** Normal dispatch in
[server.h](../include/protocol/http/v11/server.h) suppresses HEAD body bytes.
Transport-generated error responses use a separate callback/serialization
path that may not retain the request method. A handler failure may therefore
produce a HEAD response with body bytes.

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

**Status.** Verification pending; medium severity.

**Evidence and hypothesis.** In
[helpers.h](../include/protocol/http/common/helpers.h),
`split_authority_host_port()` returns the position after `]` for an IP-literal.
The absolute-form caller checks a following port only when that position
contains `:`; other trailing authority characters appear to escape rejection.

**Reproduction candidate.**

```text
GET http://[::1]junk/ HTTP/1.1\r\nHost: a\r\n\r\n
```

**Acceptance and tests.** Verify rejection in the absolute-form helper and
through request decoding. Cover valid bracketed hosts with and without a
port, malformed suffixes and fragmented input. Enforce the existing authority
grammar without adding unrelated semantic validation or rejecting permitted
syntax in adjacent forms.

**Delivery and scope.** Verify before 0.1. This concerns authority syntax,
not C4's completed absolute-form versus Host precedence fix; do not reopen C4.

<a name="b13-signed-overflow-in-the-httparena-adapter"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-dark.svg">
    <img src="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter.svg" alt="B13: Signed overflow in the HttpArena adapter">
  </picture>
</h3>

**Status.** Verification pending; medium severity, benchmark-only scope.

**Evidence and hypothesis.** In
[main.cpp](../benchmarks/httparena/http/v11/main.cpp), `read_query_sum()` adds
two parsed `int64_t` values; the POST handler then adds the parsed body value.
Successful parsing of each operand does not prove either sum is representable.
Out-of-range signed addition has undefined behavior.

**Acceptance and tests.** Exercise maximum plus one and minimum minus one
in both additions, with representable boundary sums and ordinary randomized
inputs as controls. Select a deterministic invalid-request result and verify
it with UBSan where available. Keep checks local to the adapter and preserve
the benchmark's expected arithmetic for valid inputs.

**Delivery.** Resolve before publishing the affected adapter as verified.
This is not by itself a blocker for the core library release or evidence
of arithmetic overflow inside the HTTP implementation.

<a name="product-and-convenience"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-product-and-convenience-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-product-and-convenience-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-product-and-convenience-dark.svg">
    <img src="../resources/docs/backlog/h2-product-and-convenience.svg" alt="Product and convenience">
  </picture>
</h2>

<a name="p1-static-file-handler"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-p1-static-file-handler-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p1-static-file-handler-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p1-static-file-handler-dark.svg">
    <img src="../resources/docs/backlog/h3-p1-static-file-handler.svg" alt="P1: Static file handler">
  </picture>
</h3>

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

**Status:** partial. The test expansion completed on 2026-09-07 added
184 functional cases and reinforced 84 existing cases, with 556 unit and
71 integration registrations passing the local compiler/sanitizer matrix.
The resulting bugs and capacity contract decision are tracked in
[C4](#c4-absolute-form-authority-precedence),
[C5](#c5-internal-decoder-capacity-overflow) and [C6](#c6-te-connection-option).
The count is not an exhaustive compliance claim.

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

**Remaining validation and infrastructure.**

- Force a collision with an actual spill filename candidate; check existing
  file preservation, retry, error classification and cleanup. Concurrent
  owners with distinct filenames do not establish collision handling.
- Prove that output remains pending while another client is served, and
  test the absolute send_all deadline against a non-reading local peer.
  Response serialization alone does not prove socket backpressure.
- Reproduce and resolve the gap between releasing a reserved test port and
  server bind; distinguish address-in-use from unrelated startup failures.
- Exercise a genuinely pending connect with a deterministic local mechanism
  on Windows and Linux; a refused connection does not cover that timeout.
- After C2 defines configurable limits, test zero, limit-1, limit and limit+1,
  early rejection and encoded/decoded chunked accounting.

**References.** RFC 9110 and RFC 9112. The suite provides evidence for the
strict HTTP/1.1 claim; it does not replace contract review.

<a name="qa2-fuzzing"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa2-fuzzing-dark.svg">
    <img src="../resources/docs/backlog/h3-qa2-fuzzing.svg" alt="QA2: Fuzzing">
  </picture>
</h3>

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

<a name="qa3-performance-baseline"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-qa3-performance-baseline-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa3-performance-baseline-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa3-performance-baseline-dark.svg">
    <img src="../resources/docs/backlog/h3-qa3-performance-baseline.svg" alt="QA3: Performance baseline">
  </picture>
</h3>

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

<a name="qa4-harness-diagnostics-and-isolation"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-qa4-harness-diagnostics-and-isolation-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa4-harness-diagnostics-and-isolation-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa4-harness-diagnostics-and-isolation-dark.svg">
    <img src="../resources/docs/backlog/h3-qa4-harness-diagnostics-and-isolation.svg" alt="QA4: Harness diagnostics and isolation">
  </picture>
</h3>

**Status:** completed for the stated harness criteria on 2026-09-07.

**Implemented.** Both runners identify and flush the active case, report
assertion expression/location and row context, catch standard and unknown
exceptions per case, continue afterwards, and support listing and file/name
filters. CTest bounds the suites and isolated probe verifiers. The existing
fatal-return assertion behavior and dependency-free runner are preserved.

**Evidence.** Ten named checks per runner plus an active-transport cleanup
probe passed on MSVC/GCC/Clang, Debug/Release, ASan/UBSan/TSan and CMake
3.20.6. An additional forced-suspension failure exposed and then verified
removal of a fixture leak under ASan.

**Limits.** Functional cases remain grouped into two executables. A native
crash or termination still stops its aggregate process; active-case output
and filters support focused diagnosis. This completion does not imply
per-case process isolation or exhaustive concurrency validation.

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

**Localized delivery follow-up.** The 2026-09-15 isolated-header compilation
check reported four failures with GCC 13.3:

- `include/protocol/http/common/query_parameter.h`: pair/string/view
  declarations lack their direct standard-library includes.
- `include/protocol/http/common/request_getter.h`: optional and byte-storage
  declarations depend on prior includes.
- `include/protocol/http/v11/request.h`: fails through its dependency order.
- `include/protocol/http/v11/status_lines.h`: size declarations depend on
  a preceding include.

Recheck each as the first include in a minimal C++20 translation unit using
the supported public include path, then make only the necessary dependency
fixes. Compile with strict warnings on supported compilers and preserve
installed-package consumers. The earlier check is compile-time evidence,
not a runtime test. This narrow follow-up does not make the wider platform
cleanup a release gate or require a duplicate bug entry.

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

**Context.** In [response.h](../include/protocol/http/v11/response.h), explicit
Content-Length or Transfer-Encoding can override deferred body framing.
For example, `set_body("abc")` followed by `set_header("Content-Length", "1")`
can advertise a length inconsistent with the body. An explicit chunked header
also needs a clear contract about who supplies the chunk framing.

**Open decision.** Decide whether manual framing is a caller precondition
or requires validation. State supported combinations, order of operations
and failure behavior; preserve correctly framed existing usage. Do not infer
that arbitrary manual header/body combinations were previously guaranteed.
B8 independently covers ordinary empty-body framing without manual override.

**Capacity follow-up.** Document the fixed status/header serialization budget,
space reserved for generated fields and the behavior when it is exceeded.
Test exact-fit and over-capacity cases with automatic Date/framing fields;
check that failure cannot publish a partial malformed response. Distinguish
the header budget from body storage and progressive streaming support.

**Acceptance and tests.** Cover shorter/longer explicit lengths, matching
lengths, explicit Transfer-Encoding, conflicting framing and HEAD/bodyless
statuses according to the selected contract. Verify complete serialized
output, not only individual header getters, and cross-check error recovery
with B11. Publish concise response preconditions with the chosen behavior.

**Delivery and limits.** No automatic release gate. Agree the contract before
claiming these combinations are supported. No dynamic buffers, new ownership
model, broad serializer refactor or public API change is prescribed.

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

**Outstanding work.** Document direct transport use outside `v11::server`.

**Context to preserve.** `stop()` is called outside its workers, and callbacks
do not change during start/stop. The contract distinguishes graceful closure
from fatal abort.

**Acceptance.** Verify and document call order, allowed threads, callbacks,
restart, and startup errors with public examples. Link from architecture
and validate each example against the corresponding tests.

<a name="doc2-request-views-and-getters"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-doc2-request-views-and-getters-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-doc2-request-views-and-getters-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-doc2-request-views-and-getters-dark.svg">
    <img src="../resources/docs/backlog/h3-doc2-request-views-and-getters.svg" alt="DOC2: Request views and getters">
  </picture>
</h3>

**Outstanding work.** Document string_view/header_view ownership and lifetime,
and indexed getter preconditions.

**Context to preserve.** Getters do not return owning copies; request owns
the head buffer and rebases views. A borrowed reader requires external
storage to outlive its uses.

**Acceptance.** Show valid uses, invalidation on destruction, and the
precondition chosen in DT2. Cross-check lifetime tests and link the contract
from the corresponding examples.

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

<a name="identifier-mapping"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-identifier-mapping-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-identifier-mapping-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-identifier-mapping-dark.svg">
    <img src="../resources/docs/backlog/h2-identifier-mapping.svg" alt="Identifier mapping">
  </picture>
</h2>

The previous column refers to the backlog before renumbering.
These identifiers are retained only to interpret older references;
current links and dependencies use the new column.
Completed entries retained for traceability keep their identifiers and are
marked in the inventory.

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
