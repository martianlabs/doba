<a name="backlog"></a>
<h1>
  <a href="../resources/docs/backlog/h1-backlog.svg#gh-light-mode-only">
    <img src="../resources/docs/backlog/h1-backlog.svg" alt="Backlog">
  </a>
  <a href="../resources/docs/backlog/h1-backlog-dark.svg#gh-dark-mode-only">
    <img src="../resources/docs/backlog/h1-backlog-dark.svg" alt="Backlog">
  </a>
</h1>

Source of truth for doba's outstanding implementation, verification, and
contract decisions. Identifiers remain stable when entries are removed;
numbering does not express priority or implementation order.

An outstanding item does not imply that its design has been decided.
Open decisions and deferred scope are stated explicitly. Priorities without
a selected value are marked "Not set".

<a name="contents"></a>
<h2>
  <a href="../resources/docs/backlog/h2-contents.svg#gh-light-mode-only">
    <img src="../resources/docs/backlog/h2-contents.svg" alt="Contents">
  </a>
  <a href="../resources/docs/backlog/h2-contents-dark.svg#gh-dark-mode-only">
    <img src="../resources/docs/backlog/h2-contents-dark.svg" alt="Contents">
  </a>
</h2>

- [Release target](#release-target)
- [Inventory](#inventory)
- [Operational hardening](#operational-hardening)
- [Bugs](#bugs)
- [Product and convenience](#product-and-convenience)
- [Quality and validation](#quality-and-validation)
- [Release engineering](#release-engineering)
- [C++ maintainability](#c-maintainability)
- [Beyond the first release](#beyond-the-first-release)

<a name="release-target"></a>
<h2>
  <a href="../resources/docs/backlog/h2-release-target.svg#gh-light-mode-only">
    <img src="../resources/docs/backlog/h2-release-target.svg" alt="Release target">
  </a>
  <a href="../resources/docs/backlog/h2-release-target-dark.svg#gh-dark-mode-only">
    <img src="../resources/docs/backlog/h2-release-target-dark.svg" alt="Release target">
  </a>
</h2>

The planned first delivery is `0.1.0-beta1`, a beta for controlled
deployments. There is currently no release candidate. Five open items gate
that delivery: C1-C3, C7, and RE1. GZIP is future product work.

**Implementation order.** Complete inactivity and connection admission,
settle the remaining per-request and retained-work contracts (C1/C2/C3/C7),
then finish RE1 when publication is planned. Do not delay resource controls
for optional product features.

**Exit criteria.**

- Verify C1/C2/C3/C7 with focused unit and real-socket tests on
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
  <a href="../resources/docs/backlog/h2-inventory.svg#gh-light-mode-only">
    <img src="../resources/docs/backlog/h2-inventory.svg" alt="Inventory">
  </a>
  <a href="../resources/docs/backlog/h2-inventory-dark.svg#gh-dark-mode-only">
    <img src="../resources/docs/backlog/h2-inventory-dark.svg" alt="Inventory">
  </a>
</h2>

22 open items across seven categories: five release gates and
seventeen future items. Closed and archived identifiers remain in Git history.
"Verification pending" requires a run on the relevant revision.

QA2's initial fuzzing delivery is complete: HTTP and chunked harnesses,
corpus, sanitizers, CI budgets and reproduction evidence are in
[tests/fuzz](../tests/fuzz/README.md). Broader targets or longer recurring
campaigns need separate scope. P3 is archived until a concrete middleware
composition use case is selected.

Test tooling uses repository C++ and CMake. Corpus generation and mutation
campaigns must not require Python or another scripting runtime.

| Category | Identifiers | Release | Future work | Total |
| --- | --- | --- | --- | --- |
| Operational hardening | C1-C3, C7 | 4 | 0 | 4 |
| Bugs | B13 | 0 | 1 | 1 |
| Product and convenience | F2, P2, P4-P8 | 0 | 7 | 7 |
| Quality and validation | QA1, QA6 | 0 | 2 | 2 |
| Release engineering | RE1 | 1 | 0 | 1 |
| C++ maintainability | DT1-DT3 | 0 | 3 | 3 |
| Beyond the first release | F3-F5, F7 | 0 | 4 | 4 |
| **Total** | | **5** | **17** | **22** |

| Item | Category | Status | Priority | Target |
| --- | --- | --- | --- | --- |
| [C1](#c1-single-inactivity-timeout) | Hardening | Pending | Release gate | 0.1.0-beta1 |
| [C2](#c2-effective-per-request-limits) | Hardening | Partially implemented | Release gate | 0.1.0-beta1 |
| [C3](#c3-global-active-connection-limit) | Hardening | Pending | Release gate | 0.1.0-beta1 |
| [C7](#c7-pending-response-and-work-budget) | Hardening | Partial; transport budget pending | Release gate | 0.1.0-beta1 |
| [B13](#b13-signed-overflow-in-the-httparena-adapter) | Benchmark bug | Fix and regression pending | Medium | Future work |
| [F2](#f2-compression-and-gzip) | Product | Design and implementation pending | Not set | Future work |
| [P2](#p2-access-logging) | Product | Decision pending | Not set | Future work |
| [P4](#p4-form-parsing) | Product | Pending | Not set | Future work |
| [P5](#p5-automatic-conditionals-and-ranges) | Product | Deferred | Not set | Future work |
| [P6](#p6-output-trailers) | Product | Deferred | Not set | Future work |
| [P7](#p7-automatic-resource-options) | Product | Deferred | Not set | Future work |
| [P8](#p8-429-response-construction) | Product | Pending | Low | Future work |
| [QA1](#qa1-exhaustive-compliance-suite) | QA | Pending | Not set | Future work |
| [QA6](#qa6-external-compliance-automation) | QA | Deferred | Not set | Future work |
| [RE1](#re1-release-governance-and-traceability) | Release | Partial; publication work deferred until planned | Release gate | 0.1.0-beta1 |
| [DT1](#dt1-platformh-dependencies-and-global-effects) | C++ | Pending | Medium | Future work |
| [DT2](#dt2-indexed-getter-contract) | C++ | Decision pending | Low/Medium | Future work |
| [DT3](#dt3-response-framing-and-capacity-contract) | C++ | Decision pending | Medium | Future work |
| [F3](#f3-progressive-streaming-and-sse) | Future | Deferred | Not set | Future work |
| [F4](#f4-ordered-upgrade-barrier) | Future | Deferred | Not set | Future work |
| [F5](#f5-websockets) | Future | Deferred | Not set | Future work |
| [F7](#f7-shutdown-with-draining) | Future | Deferred | Not set | Future work |

<a name="operational-hardening"></a>
<h2>
  <a href="../resources/docs/backlog/h2-operational-hardening.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-operational-hardening-narrow.svg">
      <img src="../resources/docs/backlog/h2-operational-hardening.svg" alt="Operational hardening">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h2-operational-hardening-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-operational-hardening-narrow-dark.svg">
      <img src="../resources/docs/backlog/h2-operational-hardening-dark.svg" alt="Operational hardening">
    </picture>
  </a>
</h2>

C1, C2, C3 and C7 are all required for `0.1.0-beta1`. Request and transport
policies exist; the remaining admission and resource-budget contracts still
need decisions before implementation.

<a name="c1-single-inactivity-timeout"></a>
<h3>
  <a href="../resources/docs/backlog/h3-c1-single-inactivity-timeout.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c1-single-inactivity-timeout-narrow.svg">
      <img src="../resources/docs/backlog/h3-c1-single-inactivity-timeout.svg" alt="C1: Single inactivity timeout">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-c1-single-inactivity-timeout-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c1-single-inactivity-timeout-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-c1-single-inactivity-timeout-dark.svg" alt="C1: Single inactivity timeout">
    </picture>
  </a>
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
  <a href="../resources/docs/backlog/h3-c2-effective-per-request-limits.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c2-effective-per-request-limits-narrow.svg">
      <img src="../resources/docs/backlog/h3-c2-effective-per-request-limits.svg" alt="C2: Effective per-request limits">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-c2-effective-per-request-limits-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c2-effective-per-request-limits-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-c2-effective-per-request-limits-dark.svg" alt="C2: Effective per-request limits">
    </picture>
  </a>
</h3>

**Status.** Partially implemented; required for `0.1.0-beta1`.

**Existing limits.** HTTP policies bound content length (16 MiB by default),
query pairs, chunk extensions, trailers, request-head bytes, and body storage.
Response limits are separate. A complete head over
`max_request_head_size` returns 431; an incomplete head filling that
limit currently returns 400.

**Remaining release work.** Settle consistent head-limit rejection behavior
and verify existing budgets with large receive blocks, including the work
performed before final head-size rejection. An acceptance limit alone does
not establish a bound on all transient parser work.

**Future granularity.** Separate header-field count, per-field size and
request-target knobs are optional improvements, not additional beta gates.
Define their accounting and 431/414 behavior if selected. Keep syntax
checking separate from resource policy.

**Close when.** Configured-server tests cover zero, limit-1, limit, and
limit+1, including cumulative chunked bodies and rejection before reading
an excessive declared body. Verify rejection reasons and stable configuration.
Performance measurement requires a separate explicit benchmark request.

<a name="c3-global-active-connection-limit"></a>
<h3>
  <a href="../resources/docs/backlog/h3-c3-global-active-connection-limit.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c3-global-active-connection-limit-narrow.svg">
      <img src="../resources/docs/backlog/h3-c3-global-active-connection-limit.svg" alt="C3: Global active connection limit">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-c3-global-active-connection-limit-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c3-global-active-connection-limit-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-c3-global-active-connection-limit-dark.svg" alt="C3: Global active connection limit">
    </picture>
  </a>
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
  <a href="../resources/docs/backlog/h3-c7-pending-response-and-work-budget.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-narrow.svg">
      <img src="../resources/docs/backlog/h3-c7-pending-response-and-work-budget.svg" alt="C7: Pending response and work budget">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-c7-pending-response-and-work-budget-dark.svg" alt="C7: Pending response and work budget">
    </picture>
  </a>
</h3>

**Status.** Partially implemented; required for `0.1.0-beta1`.
`max_pending_requests` bounds asynchronous response positions. TCP
`max_buffered_send_bytes` bounds queued reservations and the active batch but
excludes source-owned storage; neither is a complete retained-work budget.

**Remaining work.** Define accounting, defaults, and admission or resume
behavior for retained transport work. Distinguish queued wire bytes,
source-owned resident storage and temporary files; a source's logical
length is not its resident memory. State what remains application-owned.
Do not promise a universal budget for arbitrary allocations or external
work inside handlers. Preserve response order and transport ownership
without an HTTP-specific queue.

**Close when.** A non-reading peer and delayed first handler with pipelined
successors respect configured limits on both platforms. Verify another
connection remains serviceable, and check resume, disconnect, and stop
without leaks or duplicate completion.

<a name="bugs"></a>
<h2>
  <a href="../resources/docs/backlog/h2-bugs.svg#gh-light-mode-only">
    <img src="../resources/docs/backlog/h2-bugs.svg" alt="Bugs">
  </a>
  <a href="../resources/docs/backlog/h2-bugs-dark.svg#gh-dark-mode-only">
    <img src="../resources/docs/backlog/h2-bugs-dark.svg" alt="Bugs">
  </a>
</h2>

B13 is the only open bug recorded here. Closed fixes remain in Git history.

<a name="b13-signed-overflow-in-the-httparena-adapter"></a>
<h3>
  <a href="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-narrow.svg">
      <img src="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter.svg" alt="B13: Signed overflow in the HttpArena adapter">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-b13-signed-overflow-in-the-httparena-adapter-dark.svg" alt="B13: Signed overflow in the HttpArena adapter">
    </picture>
  </a>
</h3>

**Status.** Unfixed in the current source; benchmark-only future work.

**Problem.** `read_query_sum()` and the POST handler add parsed
`int64_t` values without checking whether the sums are representable.

**Close when.** Test maximum plus one, minimum minus one, representable
boundaries, and ordinary inputs in both additions. Define a deterministic
invalid-request result and verify with UBSan where available. Keep the fix
local to the adapter.

<a name="product-and-convenience"></a>
<h2>
  <a href="../resources/docs/backlog/h2-product-and-convenience.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-product-and-convenience-narrow.svg">
      <img src="../resources/docs/backlog/h2-product-and-convenience.svg" alt="Product and convenience">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h2-product-and-convenience-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-product-and-convenience-narrow-dark.svg">
      <img src="../resources/docs/backlog/h2-product-and-convenience-dark.svg" alt="Product and convenience">
    </picture>
  </a>
</h2>

F2, P2 and P4-P8 are future work, outside the first beta. P3 is archived
until a concrete composition use case exists.

<a name="f2-compression-and-gzip"></a>
<h3>
  <a href="../resources/docs/backlog/h3-f2-compression-and-gzip.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f2-compression-and-gzip-narrow.svg">
      <img src="../resources/docs/backlog/h3-f2-compression-and-gzip.svg" alt="F2: Compression and GZIP">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-f2-compression-and-gzip-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f2-compression-and-gzip-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-f2-compression-and-gzip-dark.svg" alt="F2: Compression and GZIP">
    </picture>
  </a>
</h3>

**Status.** Design and implementation pending; future product work.
Parsing Accept-Encoding or Content-Encoding does not implement compression.
This feature does not block resource hardening or the first beta.

**Scope.** Add GZIP response compression with `Accept-Encoding` selection,
`Content-Encoding`, and consistent `Vary`. Select the implementation and
API under the project's dependency policy. Other formats are outside scope.

**Close when.** Unit and real-socket tests on Windows and Linux verify
negotiation, identity responses, framing, and decoded empty, binary, and
large bodies.

<a name="p2-access-logging"></a>
<h3>
  <a href="../resources/docs/backlog/h3-p2-access-logging.svg#gh-light-mode-only">
    <img src="../resources/docs/backlog/h3-p2-access-logging.svg" alt="P2: Access logging">
  </a>
  <a href="../resources/docs/backlog/h3-p2-access-logging-dark.svg#gh-dark-mode-only">
    <img src="../resources/docs/backlog/h3-p2-access-logging-dark.svg" alt="P2: Access logging">
  </a>
</h3>

**Status.** Future product decision. There is no dedicated request access
log hook.

**Decide.** Choose the observation point, completion meaning, exposed peer
endpoint, status, duration, and outcome. Treat forwarded addresses as claims;
support them only with an explicit trusted-proxy policy. Do not put HTTP
semantics in the transport or assume P3 is required.

**Close when selected.** Verify success, rejection, disconnect, and send
failure records without duplicates, and define lifetime and disabled cost.

<a name="p4-form-parsing"></a>
<h3>
  <a href="../resources/docs/backlog/h3-p4-form-parsing.svg#gh-light-mode-only">
    <img src="../resources/docs/backlog/h3-p4-form-parsing.svg" alt="P4: Form parsing">
  </a>
  <a href="../resources/docs/backlog/h3-p4-form-parsing-dark.svg#gh-dark-mode-only">
    <img src="../resources/docs/backlog/h3-p4-form-parsing-dark.svg" alt="P4: Form parsing">
  </a>
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
  <a href="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges-narrow.svg">
      <img src="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges.svg" alt="P5: Automatic conditionals and ranges">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-p5-automatic-conditionals-and-ranges-dark.svg" alt="P5: Automatic conditionals and ranges">
    </picture>
  </a>
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
  <a href="../resources/docs/backlog/h3-p6-output-trailers.svg#gh-light-mode-only">
    <img src="../resources/docs/backlog/h3-p6-output-trailers.svg" alt="P6: Output trailers">
  </a>
  <a href="../resources/docs/backlog/h3-p6-output-trailers-dark.svg#gh-dark-mode-only">
    <img src="../resources/docs/backlog/h3-p6-output-trailers-dark.svg" alt="P6: Output trailers">
  </a>
</h3>

**Status.** Deferred future work. `response` has no output-trailer API.

**Scope to decide.** Support trailers only with compatible framing, field
validation, and consistent body finalization.

**Close when selected.** Verify complete wire output, terminator, field
restrictions, and unchanged output without trailers (RFC 9110 S6.5).

<a name="p7-automatic-resource-options"></a>
<h3>
  <a href="../resources/docs/backlog/h3-p7-automatic-resource-options.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p7-automatic-resource-options-narrow.svg">
      <img src="../resources/docs/backlog/h3-p7-automatic-resource-options.svg" alt="P7: Automatic resource OPTIONS">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-p7-automatic-resource-options-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p7-automatic-resource-options-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-p7-automatic-resource-options-dark.svg" alt="P7: Automatic resource OPTIONS">
    </picture>
  </a>
</h3>

**Status.** Deferred future work. `OPTIONS *` is automatic; resource
OPTIONS handlers can be registered explicitly.

**Scope to decide.** Reuse router `Allow` calculation for an optional
automatic resource response.

**Close when selected.** Preserve route precedence and define how an
explicit handler wins. Test existing and missing resources.

<a name="p8-429-response-construction"></a>
<h3>
  <a href="../resources/docs/backlog/h3-p8-429-response-construction.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p8-429-response-construction-narrow.svg">
      <img src="../resources/docs/backlog/h3-p8-429-response-construction.svg" alt="P8: 429 response construction">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-p8-429-response-construction-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-p8-429-response-construction-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-p8-429-response-construction-dark.svg" alt="P8: 429 response construction">
    </picture>
  </a>
</h3>

**Status.** Future product work. Named response status methods omit 429,
and applications cannot call the general status setter.

**Scope.** Prefer the smallest factory consistent with existing methods.
A generic status API and built-in rate limiting are separate decisions.

**Close when selected.** Verify the 429 status line, optional application-set
Retry-After, framing, and HEAD behavior without changing other factories.

<a name="quality-and-validation"></a>
<h2>
  <a href="../resources/docs/backlog/h2-quality-and-validation.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-quality-and-validation-narrow.svg">
      <img src="../resources/docs/backlog/h2-quality-and-validation.svg" alt="Quality and validation">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h2-quality-and-validation-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-quality-and-validation-narrow-dark.svg">
      <img src="../resources/docs/backlog/h2-quality-and-validation-dark.svg" alt="Quality and validation">
    </picture>
  </a>
</h2>

QA1 and QA6 are future work; QA2's initial delivery is complete. Existing
CI gates and focused regressions for response closure and C1/C2/C3/C7
remain required for `0.1.0-beta1`.

<a name="qa1-exhaustive-compliance-suite"></a>
<h3>
  <a href="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite-narrow.svg">
      <img src="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite.svg" alt="QA1: Exhaustive compliance suite">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-qa1-exhaustive-compliance-suite-dark.svg" alt="QA1: Exhaustive compliance suite">
    </picture>
  </a>
</h3>

**Status.** Existing unit and integration coverage is extensive; the
focused evidence gaps below remain future work, not a beta gate.

**Bounded scope.** This entry tracks those concrete gaps, not an open-ended
promise of exhaustive compliance. Recent concurrency, resilience and
mutation tests do not automatically close them.

**Open evidence gaps.** Force a collision with an actual spill filename;
prove one client's send remains pending while another is served and test the
`send_all` deadline; reproduce the available-port release/bind race; and
exercise a genuinely pending connect timeout.

**Close when selected.** Trace each added boundary test to an RFC rule or
project contract and verify both transports where applicable. Reuse focused
C2/C7 tests without delaying their release work.

<a name="qa6-external-compliance-automation"></a>
<h3>
  <a href="../resources/docs/backlog/h3-qa6-external-compliance-automation.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa6-external-compliance-automation-narrow.svg">
      <img src="../resources/docs/backlog/h3-qa6-external-compliance-automation.svg" alt="QA6: External compliance automation">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-qa6-external-compliance-automation-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-qa6-external-compliance-automation-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-qa6-external-compliance-automation-dark.svg" alt="QA6: External compliance automation">
    </picture>
  </a>
</h3>

**Status.** Deferred future QA work. h1spec and Http11Probe are currently
run manually.

**Close when selected.** Automate the versioned adapters in a pinned
environment, distinguish runner failures from protocol failures, and retain
logs and revisions. Coordinate with QA1.

<a name="release-engineering"></a>
<h2>
  <a href="../resources/docs/backlog/h2-release-engineering.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-release-engineering-narrow.svg">
      <img src="../resources/docs/backlog/h2-release-engineering.svg" alt="Release engineering">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h2-release-engineering-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-release-engineering-narrow-dark.svg">
      <img src="../resources/docs/backlog/h2-release-engineering-dark.svg" alt="Release engineering">
    </picture>
  </a>
</h2>

<a name="re1-release-governance-and-traceability"></a>
<h3>
  <a href="../resources/docs/backlog/h3-re1-release-governance-and-traceability.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-re1-release-governance-and-traceability-narrow.svg">
      <img src="../resources/docs/backlog/h3-re1-release-governance-and-traceability.svg" alt="RE1: Release governance and traceability">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-re1-release-governance-and-traceability-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-re1-release-governance-and-traceability-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-re1-release-governance-and-traceability-dark.svg" alt="RE1: Release governance and traceability">
    </picture>
  </a>
</h3>

**Status.** Partially implemented; release gate when publication of
`0.1.0-beta1` is planned. There is currently no candidate to qualify.

**Already implemented.** CI covers compiler/configuration builds, Release
tests, TLS, sanitizers, minimum CMake, isolated package/source consumers,
test inventories and retained diagnostics. Fuzzing, resilience and mutation
detection are also integrated. Usage guides and API reference pages exist.

**Remaining scope.** Review concrete usage preconditions and deployment
limitations in the existing documentation, especially indexed getter
preconditions and manual response framing. Before publication, complete
release notes, an available vulnerability-reporting channel and coherent
versioning/prerelease publication. The workflow still derives only
vMAJOR.MINOR.PATCH and does not pass --prerelease. The full contribution
guide remains future work.

**Close when publication is planned.** C1/C2/C3/C7 pass focused tests; the exact release
revision passes CI and CMake consumer checks. Document indexed-getter and
view lifetimes, response framing and capacity, transport lifecycle, IPv4-only
scope, TLS configuration, and known limitations, including absent built-in
GZIP compression. Align the tested
content, notes, and `v0.1.0-beta1` tag; publish as a prerelease.

<a name="c-maintainability"></a>
<h2>
  <a href="../resources/docs/backlog/h2-c-maintainability.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-c-maintainability-narrow.svg">
      <img src="../resources/docs/backlog/h2-c-maintainability.svg" alt="C++ maintainability">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h2-c-maintainability-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-c-maintainability-narrow-dark.svg">
      <img src="../resources/docs/backlog/h2-c-maintainability-dark.svg" alt="C++ maintainability">
    </picture>
  </a>
</h2>

DT1-DT3 are future work. RE1 covers the minimum documentation of current
usage preconditions for `0.1.0-beta1`; these entries do not require API
changes or a broader redesign before that release.

<a name="dt1-platformh-dependencies-and-global-effects"></a>
<h3>
  <a href="../resources/docs/backlog/h3-dt1-platformh-dependencies-and-global-effects.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-dt1-platformh-dependencies-and-global-effects-narrow.svg">
      <img src="../resources/docs/backlog/h3-dt1-platformh-dependencies-and-global-effects.svg" alt="DT1: platform.h dependencies and global effects">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-dt1-platformh-dependencies-and-global-effects-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-dt1-platformh-dependencies-and-global-effects-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-dt1-platformh-dependencies-and-global-effects-dark.svg" alt="DT1: platform.h dependencies and global effects">
    </picture>
  </a>
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
  <a href="../resources/docs/backlog/h3-dt2-indexed-getter-contract.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-dt2-indexed-getter-contract-narrow.svg">
      <img src="../resources/docs/backlog/h3-dt2-indexed-getter-contract.svg" alt="DT2: Indexed getter contract">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-dt2-indexed-getter-contract-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-dt2-indexed-getter-contract-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-dt2-indexed-getter-contract-dark.svg" alt="DT2: Indexed getter contract">
    </picture>
  </a>
</h3>

**Status.** Future contract decision.

**Problem.** `request::get_header(size_t)` and
`get_query_parameter(size_t)` use unchecked indexing; callers currently
need valid indices.

**Smallest remaining task.** Document the current precondition
0 <= i < get_headers_length() or get_query_parameters_length(), as
appropriate. A checked API is a separate optional change, not required
to document current behavior. Do not test out-of-range unchecked access
as though it were a supported operation.

<a name="dt3-response-framing-and-capacity-contract"></a>
<h3>
  <a href="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract-narrow.svg">
      <img src="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract.svg" alt="DT3: Response framing and capacity contract">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-dt3-response-framing-and-capacity-contract-dark.svg" alt="DT3: Response framing and capacity contract">
    </picture>
  </a>
</h3>

**Status.** Future contract decision, not a confirmed independent bug.

**Already covered.** Response tests exercise conflicting CL/TE rejection,
deferred framing, Date generation/removal and exact/exceeded head capacity.
Those cases are not missing implementation work.

**Problem.** Manual Content-Length or Transfer-Encoding can override
deferred response framing. Define caller preconditions or validation,
supported combinations, operation order, and failure behavior.

**Close when selected.** Define and test shorter/longer explicit lengths,
manual Transfer-Encoding and their ordering/ownership preconditions.
Verify failure recovery for any newly rejected combination. Preserve the
existing Date and capacity checks; keep head capacity distinct from body
storage and avoid an unrelated serializer redesign.

<a name="beyond-the-first-release"></a>
<h2>
  <a href="../resources/docs/backlog/h2-beyond-the-first-release.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-beyond-the-first-release-narrow.svg">
      <img src="../resources/docs/backlog/h2-beyond-the-first-release.svg" alt="Beyond the first release">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h2-beyond-the-first-release-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h2-beyond-the-first-release-narrow-dark.svg">
      <img src="../resources/docs/backlog/h2-beyond-the-first-release-dark.svg" alt="Beyond the first release">
    </picture>
  </a>
</h2>

F3-F5 and F7 are future features outside `0.1.0-beta1`.

<a name="f3-progressive-streaming-and-sse"></a>
<h3>
  <a href="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse-narrow.svg">
      <img src="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse.svg" alt="F3: Progressive streaming and SSE">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-f3-progressive-streaming-and-sse-dark.svg" alt="F3: Progressive streaming and SSE">
    </picture>
  </a>
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
  <a href="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier-narrow.svg">
      <img src="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier.svg" alt="F4: Ordered upgrade barrier">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-f4-ordered-upgrade-barrier-dark.svg" alt="F4: Ordered upgrade barrier">
    </picture>
  </a>
</h3>

**Status.** Deferred future feature; prerequisite for F5.

**Scope to decide.** Deliver the 101 response in order before transferring
the channel, stop HTTP decoding at that boundary, and pass residual bytes
to the new codec. Keep the contract independent of IOCP and epoll.

**Close when selected.** Test previous responses, already received bytes,
closure, and errors with a dummy codec.

<a name="f5-websockets"></a>
<h3>
  <a href="../resources/docs/backlog/h3-f5-websockets.svg#gh-light-mode-only">
    <img src="../resources/docs/backlog/h3-f5-websockets.svg" alt="F5: WebSockets">
  </a>
  <a href="../resources/docs/backlog/h3-f5-websockets-dark.svg#gh-dark-mode-only">
    <img src="../resources/docs/backlog/h3-f5-websockets-dark.svg" alt="F5: WebSockets">
  </a>
</h3>

**Status.** Deferred future feature. Upgrade intent and
Sec-WebSocket-* headers are modeled, but no transport upgrade occurs.

**Scope to decide.** Handshake, framing, fragmentation, ping/pong/close,
externally initiated sends, and bidirectional backpressure depend on F4.

**Close when selected.** Define the protocol contract and test fragmented
messages, simultaneous closure, errors, and slow clients on both platforms.

<a name="f7-shutdown-with-draining"></a>
<h3>
  <a href="../resources/docs/backlog/h3-f7-shutdown-with-draining.svg#gh-light-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f7-shutdown-with-draining-narrow.svg">
      <img src="../resources/docs/backlog/h3-f7-shutdown-with-draining.svg" alt="F7: Shutdown with draining">
    </picture>
  </a>
  <a href="../resources/docs/backlog/h3-f7-shutdown-with-draining-dark.svg#gh-dark-mode-only">
    <picture>
      <source media="(max-width: 640px)" srcset="../resources/docs/backlog/h3-f7-shutdown-with-draining-narrow-dark.svg">
      <img src="../resources/docs/backlog/h3-f7-shutdown-with-draining-dark.svg" alt="F7: Shutdown with draining">
    </picture>
  </a>
</h3>

**Status.** Deferred future feature. Current `stop()` does not offer a
bounded server-wide drain and waits for externally suspended handlers
without a cancellation deadline.

**Existing behavior.** Quiesce, queued-output drain and waiting for pending
handlers are implemented and tested. The remaining feature is a bounded
shutdown deadline and cancellation policy, not the introduction of draining.

**Scope to decide.** Define when acceptance and request admission stop,
how active work drains, and what happens at a deadline without silently
changing existing `stop()` semantics.

**Close when selected.** Test active sends, idle keep-alive, pipelining,
pending output, expiry, repeated stop, and single cleanup on both backends.
