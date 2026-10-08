<a name="backlog"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h1-backlog-dark.svg">
    <img src="../resources/docs/backlog/h1-backlog.svg" alt="Backlog">
  </picture>
</h1>

Source of truth for doba's outstanding implementation, verification, and
contract decisions. Identifiers remain stable when entries are removed;
numbering does not express priority or implementation order.

An outstanding item does not imply that its design has been decided.
Open decisions and deferred scope are stated explicitly. Priorities without
a selected value are marked "Not set".

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

The frozen target is `0.1.0-beta1`, a beta for controlled
deployments. Seven open items gate it: C1-C3, C7, F1-F2, and RE1.

**Implementation order.** Finish TLS validation and GZIP compression (F1/F2);
complete inactivity, per-request, connection, and pending-work limits
(C1/C2/C3/C7); then finish release engineering (RE1).

**Exit criteria.**

- Verify F1/F2 and C1/C2/C3/C7 with focused unit and real-socket tests on
  Windows and Linux where applicable. Include slow clients, ordering,
  resource release, and cancellation where relevant.
- Pass the compiler/configuration matrix, strict warnings, sanitizers,
  minimum CMake, and isolated package and source consumers on the exact
  release revision.
- Publish minimum usage preconditions, deployment limitations, release notes,
  an available vulnerability-reporting channel, and a consistent
  `v0.1.0-beta1` prerelease.

Future work does not block this beta. Adding another release gate requires
an explicit scope decision supported by evidence.

<a name="inventory"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-inventory-dark.svg">
    <img src="../resources/docs/backlog/h2-inventory.svg" alt="Inventory">
  </picture>
</h2>

29 open items across eight categories: seven release gates and
twenty-two future items. Closed identifiers remain in Git history.
"Verification pending" requires a run on the relevant revision.

| Category | Identifiers | Release | Future work | Total |
| --- | --- | --- | --- | --- |
| Operational hardening | C1-C3, C7 | 4 | 0 | 4 |
| Bugs | B13 | 0 | 1 | 1 |
| Product and convenience | F1-F2, P2-P8 | 2 | 7 | 9 |
| Quality and validation | QA1-QA3, QA5-QA6 | 0 | 5 | 5 |
| Release engineering | RE1 | 1 | 0 | 1 |
| C++ maintainability | DT1-DT3 | 0 | 3 | 3 |
| Public documentation | DOC1-DOC2 | 0 | 2 | 2 |
| Beyond the first release | F3-F5, F7 | 0 | 4 | 4 |
| **Total** | | **7** | **22** | **29** |

| Item | Category | Status | Priority | Target |
| --- | --- | --- | --- | --- |
| [C1](#c1-single-inactivity-timeout) | Hardening | Pending | Release gate | 0.1.0-beta1 |
| [C2](#c2-effective-per-request-limits) | Hardening | Partially implemented | Release gate | 0.1.0-beta1 |
| [C3](#c3-global-active-connection-limit) | Hardening | Pending | Release gate | 0.1.0-beta1 |
| [C7](#c7-pending-response-and-work-budget) | Hardening | Partial; transport budget pending | Release gate | 0.1.0-beta1 |
| [B13](#b13-signed-overflow-in-the-httparena-adapter) | Benchmark bug | Verification pending | Medium | Future work |
| [F1](#f1-tls) | Product | Implemented; release validation pending | Release gate | 0.1.0-beta1 |
| [F2](#f2-compression-and-gzip) | Product | Design and implementation pending | Release gate | 0.1.0-beta1 |
| [P2](#p2-access-logging) | Product | Decision pending | Not set | Future work |
| [P3](#p3-middleware-chain) | Product | Decision pending | Not set | Future work |
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
| [DT2](#dt2-indexed-getter-contract) | C++ | Decision pending | Low/Medium | Future work |
| [DT3](#dt3-response-framing-and-capacity-contract) | C++ | Decision pending | Medium | Future work |
| [DOC1](#doc1-transport-lifecycle) | Documentation | Pending | Not set | Future work |
| [DOC2](#doc2-request-views-and-getters) | Documentation | Pending | Not set | Future work |
| [F3](#f3-progressive-streaming-and-sse) | Future | Deferred | Not set | Future work |
| [F4](#f4-ordered-upgrade-barrier) | Future | Deferred | Not set | Future work |
| [F5](#f5-websockets) | Future | Deferred | Not set | Future work |
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

C1, C2, C3 and C7 are all required for `0.1.0-beta1`. Request and transport
policies exist; the remaining admission and resource-budget contracts still
need decisions before implementation.

<a name="c1-single-inactivity-timeout"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c1-single-inactivity-timeout-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c1-single-inactivity-timeout-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c1-single-inactivity-timeout-dark.svg">
    <img src="../resources/docs/backlog/h3-c1-single-inactivity-timeout.svg" alt="C1: Single inactivity timeout">
  </picture>
</h3>

**Status.** Pending; required for `0.1.0-beta1`. Neither TCP backend
closes a connection that stops making progress.

**Scope.** Add one transport-policy `inactivity_timeout` fixed at creation.
Receiving or sending bytes renews the deadline; expiry closes the connection.
Decide the default and disabling behavior before implementation. Phase-specific
and absolute deadlines, dynamic changes, and automatic 408 responses are
outside this item.

**Close when.** Fragmented reads that progress stay alive; stalled partial
requests, idle keep-alive, and non-reading peers expire. Verify timing,
ownership, response order, and one disconnect callback on IOCP and epoll.

<a name="c2-effective-per-request-limits"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c2-effective-per-request-limits-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c2-effective-per-request-limits-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c2-effective-per-request-limits-dark.svg">
    <img src="../resources/docs/backlog/h3-c2-effective-per-request-limits.svg" alt="C2: Effective per-request limits">
  </picture>
</h3>

**Status.** Partially implemented; required for `0.1.0-beta1`.

**Existing limits.** HTTP policies bound content length (16 MiB by default),
query pairs, chunk extensions, trailers, request-head bytes, and body storage.
Response capacities are separate. A complete head over
`max_request_head_size` returns 431; an incomplete head filling that limit
currently returns 400.

**Remaining work.** Define separate header-field count, per-field size, and
request-target limits, including exact accounting and 431/414 rejection
behavior. Keep syntax checking separate from resource policy.

**Close when.** Configured-server tests cover zero, limit-1, limit, and
limit+1, including cumulative chunked bodies and rejection before reading
an excessive declared body. Verify rejection reasons and stable configuration.
Performance measurement requires a separate explicit benchmark request.

<a name="c3-global-active-connection-limit"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c3-global-active-connection-limit-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c3-global-active-connection-limit-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c3-global-active-connection-limit-dark.svg">
    <img src="../resources/docs/backlog/h3-c3-global-active-connection-limit.svg" alt="C3: Global active connection limit">
  </picture>
</h3>

**Status.** Pending; required for `0.1.0-beta1`. The current connection
counter observes connections but does not limit admission.

**Scope.** Add one global transport-policy maximum fixed at creation. Reserve
capacity before admitting a context; close excess connections without
interrupting admitted ones. Decide the default and unlimited value first.
Per-worker quotas, dynamic changes, and HTTP rejection responses are outside
this item.

**Close when.** Concurrent-client tests on IOCP and epoll never exceed the
maximum, release each reservation on every closure path, and admit a new
connection after capacity becomes available.

<a name="c7-pending-response-and-work-budget"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-dark.svg">
    <img src="../resources/docs/backlog/h3-c7-pending-response-and-work-budget.svg" alt="C7: Pending response and work budget">
  </picture>
</h3>

**Status.** Partially implemented; required for `0.1.0-beta1`.
`max_pending_requests` bounds asynchronous response positions. TCP
`max_send_buffer_size` bounds queued reservations and the active batch but
excludes source-owned storage; neither is a complete retained-work budget.

**Remaining work.** Define accounting, defaults, and admission or resume
behavior for queued response sources and other retained transport work.
Preserve response order and transport ownership without an HTTP-specific
queue.

**Close when.** A non-reading peer and delayed first handler with pipelined
successors respect configured limits on both platforms. Verify another
connection remains serviceable, and check resume, disconnect, and stop
without leaks or duplicate completion.

<a name="bugs"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-bugs-dark.svg">
    <img src="../resources/docs/backlog/h2-bugs.svg" alt="Bugs">
  </picture>
</h2>

B13 is the only open bug recorded here. Closed fixes remain in Git history.

<a name="b13-signed-overflow-in-the-httparena-adapter"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-dark.svg">
    <img src="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter.svg" alt="B13: Signed overflow in the HttpArena adapter">
  </picture>
</h3>

**Status.** Verification pending; benchmark-only future work.

**Problem.** `read_query_sum()` and the POST handler add parsed
`int64_t` values without checking whether the sums are representable.

**Close when.** Test maximum plus one, minimum minus one, representable
boundaries, and ordinary inputs in both additions. Define a deterministic
invalid-request result and verify with UBSan where available. Keep the fix
local to the adapter.

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

**Status.** Implemented; release validation remains open.
`DOBA_ENABLE_TLS=ON` builds `doba_tls` with optional OpenSSL support.
Linux real-socket tests exist. CI is configured to build and test TLS on
Windows Release; a passing result for the target revision is not established
by that configuration alone.

**Close when.** Run the enabled-TLS unit and real-socket suites on Windows
and Linux, including malformed records, handshake, encrypted HTTP delivery,
ordering, closure, and errors. Complete the release matrix before closing F1.

<a name="f2-compression-and-gzip"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-f2-compression-and-gzip-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f2-compression-and-gzip-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f2-compression-and-gzip-dark.svg">
    <img src="../resources/docs/backlog/h3-f2-compression-and-gzip.svg" alt="F2: Compression and GZIP">
  </picture>
</h3>

**Status.** Design and implementation pending; required for
`0.1.0-beta1`.

**Scope.** Add GZIP response compression with `Accept-Encoding` selection,
`Content-Encoding`, and consistent `Vary`. Select the implementation and
API under the project's dependency policy. Other formats are outside scope.

**Close when.** Unit and real-socket tests on Windows and Linux verify
negotiation, identity responses, framing, and decoded empty, binary, and
large bodies.

<a name="p2-access-logging"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p2-access-logging-dark.svg">
    <img src="../resources/docs/backlog/h3-p2-access-logging.svg" alt="P2: Access logging">
  </picture>
</h3>

**Status.** Future product decision. There is no dedicated request access
log hook.

**Decide.** Choose the observation point, completion meaning, exposed peer
endpoint, status, duration, and outcome. Treat forwarded addresses as claims;
support them only with an explicit trusted-proxy policy. Do not put HTTP
semantics in the transport or assume P3 is required.

**Close when selected.** Verify success, rejection, disconnect, and send
failure records without duplicates, and define lifetime and disabled cost.

<a name="p3-middleware-chain"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-p3-middleware-chain-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p3-middleware-chain-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p3-middleware-chain-dark.svg">
    <img src="../resources/docs/backlog/h3-p3-middleware-chain.svg" alt="P3: Middleware chain">
  </picture>
</h3>

**Status.** Future product decision; no concrete composition use case has
been selected.

**Decide.** Identify a real need across synchronous and asynchronous
handlers before defining middleware. A general extension framework is not
justified by this entry.

**Close when selected.** Specify ordering, short-circuiting, errors,
cancellation, and ownership; test those contracts and the path without
middleware.

<a name="p4-form-parsing"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p4-form-parsing-dark.svg">
    <img src="../resources/docs/backlog/h3-p4-form-parsing.svg" alt="P4: Form parsing">
  </picture>
</h3>

**Status.** Future product work. Form parsing is absent.

**Scope to decide.** Treat `application/x-www-form-urlencoded` and
`multipart/form-data` separately. Define repeated fields, binary payloads,
errors, and limits without assuming query-string semantics.

**Close when selected.** Test valid and malformed input, encoding,
empty and repeated fields, resource limits, and fragmented multipart
boundaries. Coordinate budgets with C2.

<a name="p5-automatic-conditionals-and-ranges"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges-dark.svg">
    <img src="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges.svg" alt="P5: Automatic conditionals and ranges">
  </picture>
</h3>

**Status.** Deferred future work. The static file server handles
existence-based `If-Match: *` and `If-None-Match: *` but supplies no entity
tags, date validators, or byte ranges.

**Scope to decide.** Add optional helpers using application-supplied
representation metadata. Preserve wildcard conditions and the option to
ignore Range and serve a normal GET.

**Close when selected.** Define metadata ownership and precondition order;
test added validators and any supported selected or unsatisfied ranges
(RFC 9110 S13.2 and S14).

<a name="p6-output-trailers"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p6-output-trailers-dark.svg">
    <img src="../resources/docs/backlog/h3-p6-output-trailers.svg" alt="P6: Output trailers">
  </picture>
</h3>

**Status.** Deferred future work. `response` has no output-trailer API.

**Scope to decide.** Support trailers only with compatible framing, field
validation, and consistent body finalization.

**Close when selected.** Verify complete wire output, terminator, field
restrictions, and unchanged output without trailers (RFC 9110 S6.5).

<a name="p7-automatic-resource-options"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-p7-automatic-resource-options-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p7-automatic-resource-options-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p7-automatic-resource-options-dark.svg">
    <img src="../resources/docs/backlog/h3-p7-automatic-resource-options.svg" alt="P7: Automatic resource OPTIONS">
  </picture>
</h3>

**Status.** Deferred future work. `OPTIONS *` is automatic; resource
OPTIONS handlers can be registered explicitly.

**Scope to decide.** Reuse router `Allow` calculation for an optional
automatic resource response.

**Close when selected.** Preserve route precedence and define how an
explicit handler wins. Test existing and missing resources.

<a name="p8-429-response-construction"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-p8-429-response-construction-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p8-429-response-construction-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-p8-429-response-construction-dark.svg">
    <img src="../resources/docs/backlog/h3-p8-429-response-construction.svg" alt="P8: 429 response construction">
  </picture>
</h3>

**Status.** Future product work. Named response status methods omit 429,
and applications cannot call the general status setter.

**Scope.** Prefer the smallest factory consistent with existing methods.
A generic status API and built-in rate limiting are separate decisions.

**Close when selected.** Verify the 429 status line, optional application-set
Retry-After, framing, and HEAD behavior without changing other factories.

<a name="quality-and-validation"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-quality-and-validation-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-quality-and-validation-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-quality-and-validation-dark.svg">
    <img src="../resources/docs/backlog/h2-quality-and-validation.svg" alt="Quality and validation">
  </picture>
</h2>

QA1-QA3 and QA5-QA6 are future work. Existing CI gates and focused
regressions for response closure, F1/F2, and C1/C2/C3/C7 remain required
for `0.1.0-beta1`.

<a name="qa1-exhaustive-compliance-suite"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite-dark.svg">
    <img src="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite.svg" alt="QA1: Exhaustive compliance suite">
  </picture>
</h3>

**Status.** Future QA work; not a beta gate.

**Open evidence gaps.** Force a collision with an actual spill filename;
prove one client's send remains pending while another is served and test the
`send_all` deadline; reproduce the available-port release/bind race; and
exercise a genuinely pending connect timeout.

**Close when selected.** Trace each added boundary test to an RFC rule or
project contract and verify both transports where applicable. Reuse focused
C2/C7 tests without delaying their release work.

<a name="qa2-fuzzing"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa2-fuzzing-dark.svg">
    <img src="../resources/docs/backlog/h3-qa2-fuzzing.svg" alt="QA2: Fuzzing">
  </picture>
</h3>

**Status.** Deferred future QA work. No fuzz targets or corpus are present.

**Scope to decide.** Select targets across HTTP helpers, decoding, framing,
request rebasing, serialization, storage, and relevant transport boundaries.
Choose infrastructure before adding dependencies.

**Close when selected.** Define corpus, budgets, sanitizer use, and failure
reproduction; turn confirmed failures into deterministic regressions.

<a name="qa3-performance-baseline"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-qa3-performance-baseline-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa3-performance-baseline-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa3-performance-baseline-dark.svg">
    <img src="../resources/docs/backlog/h3-qa3-performance-baseline.svg" alt="QA3: Performance baseline">
  </picture>
</h3>

**Status.** Future QA work. Published HttpArena baseline and pipelined
results contain one repetition, so they do not establish variability or
regression tolerances. The Web Frameworks adapter already fetches a pinned
commit.

**Remaining work.** Define representative scenarios, hardware, toolchain,
revisions, resource metrics, repeated samples, and tolerances. Preserve the
inputs and results needed to compare a release candidate. Keep measurement
separate from optimization.

<a name="qa5-stress-campaigns"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-qa5-stress-campaigns-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa5-stress-campaigns-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa5-stress-campaigns-dark.svg">
    <img src="../resources/docs/backlog/h3-qa5-stress-campaigns.svg" alt="QA5: Stress campaigns">
  </picture>
</h3>

**Status.** Future QA work. Functional integration and concurrency cases
exist; longer soak runs and controlled worker interleavings remain undefined.

**Close when selected.** Specify duration, load, observed resources, and
failure reproduction before adding tests. Relate scenarios to C1, C3, and
QA3.

<a name="qa6-external-compliance-automation"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-qa6-external-compliance-automation-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa6-external-compliance-automation-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-qa6-external-compliance-automation-dark.svg">
    <img src="../resources/docs/backlog/h3-qa6-external-compliance-automation.svg" alt="QA6: External compliance automation">
  </picture>
</h3>

**Status.** Deferred future QA work. h1spec and Http11Probe are currently
run manually.

**Close when selected.** Automate the versioned adapters in a pinned
environment, distinguish runner failures from protocol failures, and retain
logs and revisions. Coordinate with QA1.

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

**Status.** Release gate for `0.1.0-beta1`. The publishing workflow derives
only `vMAJOR.MINOR.PATCH` from `include/version.h`.

**Scope.** Complete release notes, an available vulnerability-reporting
channel, minimum usage and deployment documentation, coherent versioning,
and prerelease publication. The full contribution guide remains future work.

**Close when.** F1/F2 and C1/C2/C3/C7 pass focused tests; the exact release
revision passes CI and CMake consumer checks. Document indexed-getter and
view lifetimes, response framing and capacity, transport lifecycle, IPv4-only
scope, TLS/GZIP configuration, and known limitations. Align the tested
content, notes, and `v0.1.0-beta1` tag; publish as a prerelease.

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

**Status.** Future C++ maintenance work.

**Problem.** `platform.h` exposes Windows macros, warning suppression,
linker pragmas, and transitive includes. `tcp.h` has no explicit branch for
unsupported platforms.

**Close when selected.** Inventory and justify each consumer-visible effect,
check public headers independently, and verify any local change with installed
package consumers. Do not broaden platform support or reorganize includes
without evidence.

<a name="dt2-indexed-getter-contract"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-dt2-indexed-getter-contract-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-dt2-indexed-getter-contract-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-dt2-indexed-getter-contract-dark.svg">
    <img src="../resources/docs/backlog/h3-dt2-indexed-getter-contract.svg" alt="DT2: Indexed getter contract">
  </picture>
</h3>

**Status.** Future contract decision.

**Problem.** `request::get_header(size_t)` and
`get_query_parameter(size_t)` use unchecked indexing; callers currently
need valid indices.

**Close when selected.** Decide between a documented precondition and a
checked API, then test its boundaries and preserve compatibility. Coordinate
the documentation with DOC2; inconsistency alone does not require an API
change.

<a name="dt3-response-framing-and-capacity-contract"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract-dark.svg">
    <img src="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract.svg" alt="DT3: Response framing and capacity contract">
  </picture>
</h3>

**Status.** Future contract decision, not a confirmed independent bug.

**Problem.** Manual Content-Length or Transfer-Encoding can override
deferred response framing. Define caller preconditions or validation,
supported combinations, operation order, and failure behavior.

**Close when selected.** Test shorter and longer explicit lengths, manual
Transfer-Encoding, generated Date and framing at exact and exceeded header
capacity, and recovery without partial malformed output. Keep header
capacity distinct from body storage; avoid an unrelated serializer redesign.

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

**Status.** Future documentation work beyond RE1's minimum.

**Scope.** Explain direct transport use in README or examples: callback
setup, start/stop order, worker-thread restrictions, restart, and startup
errors. Distinguish per-connection closure from F7's proposed server-wide
bounded drain.

**Close when selected.** Validate examples against transport lifecycle tests
and link them from the public usage material.

<a name="doc2-request-views-and-getters"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-doc2-request-views-and-getters-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-doc2-request-views-and-getters-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-doc2-request-views-and-getters-dark.svg">
    <img src="../resources/docs/backlog/h3-doc2-request-views-and-getters.svg" alt="DOC2: Request views and getters">
  </picture>
</h3>

**Status.** Future documentation work beyond RE1's minimum.

**Scope.** Use README or examples to show safe request views across
coroutine suspension, invalidation on destruction, borrowed-reader lifetime,
and indexed-getter preconditions.

**Close when selected.** Cross-check request and reader tests, document the
DT2 decision, and link the examples from public usage material.

<a name="beyond-the-first-release"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h2-beyond-the-first-release-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-beyond-the-first-release-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h2-beyond-the-first-release-dark.svg">
    <img src="../resources/docs/backlog/h2-beyond-the-first-release.svg" alt="Beyond the first release">
  </picture>
</h2>

F3-F5 and F7 are future features outside `0.1.0-beta1`.

<a name="f3-progressive-streaming-and-sse"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse-dark.svg">
    <img src="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse.svg" alt="F3: Progressive streaming and SSE">
  </picture>
</h3>

**Status.** Deferred future feature. Current handlers finish producing
responses before handoff; transport draining is not progressive streaming.

**Scope to decide.** Represent response start, fragments, completion and
errors with byte-based backpressure.

**Close when selected.** Test cancellation, slow clients, partial errors,
and response order without regressing the one-shot path. Benchmarking needs
a separate explicit request.

<a name="f4-ordered-upgrade-barrier"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier-dark.svg">
    <img src="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier.svg" alt="F4: Ordered upgrade barrier">
  </picture>
</h3>

**Status.** Deferred future feature; prerequisite for F5.

**Scope to decide.** Deliver the 101 response in order before transferring
the channel, stop HTTP decoding at that boundary, and pass residual bytes
to the new codec. Keep the contract independent of IOCP and epoll.

**Close when selected.** Test previous responses, already received bytes,
closure, and errors with a dummy codec.

<a name="f5-websockets"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f5-websockets-dark.svg">
    <img src="../resources/docs/backlog/h3-f5-websockets.svg" alt="F5: WebSockets">
  </picture>
</h3>

**Status.** Deferred future feature. Upgrade intent and
Sec-WebSocket-* headers are modeled, but no transport upgrade occurs.

**Scope to decide.** Handshake, framing, fragmentation, ping/pong/close,
externally initiated sends, and bidirectional backpressure depend on F4.

**Close when selected.** Define the protocol contract and test fragmented
messages, simultaneous closure, errors, and slow clients on both platforms.

<a name="f7-shutdown-with-draining"></a>
<h3>
  <picture>
    <source media="(prefers-color-scheme: dark) and (max-width: 640px)" srcset="../resources/docs/backlog/h3-f7-shutdown-with-draining-narrow-dark.svg">
    <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f7-shutdown-with-draining-narrow.svg">
    <source media="(prefers-color-scheme: dark)" srcset="../resources/docs/backlog/h3-f7-shutdown-with-draining-dark.svg">
    <img src="../resources/docs/backlog/h3-f7-shutdown-with-draining.svg" alt="F7: Shutdown with draining">
  </picture>
</h3>

**Status.** Deferred future feature. Current `stop()` does not offer a
bounded server-wide drain and waits for externally suspended handlers
without a cancellation deadline.

**Scope to decide.** Define when acceptance and request admission stop,
how active work drains, and what happens at a deadline without silently
changing existing `stop()` semantics.

**Close when selected.** Test active sends, idle keep-alive, pipelining,
pending output, expiry, repeated stop, and single cleanup on both backends.
Coordinate user-facing lifecycle guidance with DOC1.
