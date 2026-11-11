<a name="policy-checks"></a>
<h1>
  <a href="../../resources/tests/qualification/policies/h1-policy-checks.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h1-policy-checks.svg" alt="Policy checks">
  </a>
  <a href="../../resources/tests/qualification/policies/h1-policy-checks-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h1-policy-checks-dark.svg" alt="Policy checks">
  </a>
</h1>

This inventory follows the public fields, rather than counting tests that
only read defaults back. A limit needs an observable effect: admission,
rejection, bounded work, storage choice or exact output.

<a name="tcp"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-tcp.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-tcp.svg" alt="TCP">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-tcp-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-tcp-dark.svg" alt="TCP">
  </a>
</h2>

Paths below start at `tests/`.

| Field | Behavioral checks |
| --- | --- |
| `receive_buffer_capacity` | `integration/transport/tcp_policy_limits_tests.cpp`: three-byte input blocks and binary echo under full admission. Zero and native numeric bounds in `unit/transport/tcp_transport_tests.cpp` and native policy tests. |
| `worker_count` | Native Linux/Windows connection and processing tests, automatic selection, independent connections; policy saturation with one/four workers. |
| `listen_backlog` | Real listeners with 0, 1 and INT_MAX in `tcp_policy_limits_tests.cpp`; negative values rejected. The kernel may lower the requested backlog. |
| `max_buffered_send_bytes` | Exact capacity and one-byte overflow on real sockets; `unit/transport/operating_policies_tests.cpp` keeps the whole active batch charged through partial sends and bounds reader blocks. |
| `max_connections` | Zero, finite rejection before engine creation, factory failure and slot release. Saturation keeps sixteen live peers, rejects 32 concurrent excess peers, checks binary replies and refills freed slots over eight cycles with one/four workers. Linux worker partitions are allowed to reject early while filling; a full server must reject every excess peer. |
| `ip` | Valid loopback and malformed addresses, including embedded/trailing NUL, in `tcp_policy_limits_tests.cpp`. |
| `port` | Valid ephemeral port, zero, range overflow, signs, whitespace and embedded/trailing NUL in the same file. |

<a name="tls"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-tls.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-tls.svg" alt="TLS">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-tls-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-tls-dark.svg" alt="TLS">
  </a>
</h2>

TCP settings are inherited. `encrypted_receive_buffer_capacity` becomes the
TCP RX block size; `max_buffered_ciphertext_bytes` becomes the TCP TX budget.
The plaintext settings remain separate inside the TLS engine.

| Field | Behavioral checks |
| --- | --- |
| `certificate_file` | Verified native clients, missing/invalid certificate, embedded NUL and mismatch in `integration/tls/configuration.cpp` and `exercise.cpp`. |
| `private_key_file` | Matching, missing, malformed, mismatched and encrypted keys; no password prompt. |
| `encrypted_receive_buffer_capacity` | Zero rejected; native TLS exchanges and source/close with one/seventeen-byte encrypted RX and three-byte plaintext RX in `exercise.cpp`. |
| `network_bio_buffer_capacity` | Zero and INT_MAX overflow rejected. Real progress with capacities 1/127/17408 in `core.cpp` and `processing.cpp`. |
| `max_buffered_ciphertext_bytes` | Zero rejected. `processing.cpp` measures a valid encrypted batch, then checks size−1, size and size+1 for both TLS versions and each BIO capacity. Overflow emits no partial batch and cannot revive processing. Native overflow closes one connection and a fresh client still succeeds. |
| `alpn_protocol` | Empty and 255-byte configuration accepted, 256 rejected; native exact match, absent/mismatched offers and incompatible HTTP ALPN in `exercise.cpp`. |

Plaintext RX is asserted at engine entry. Plaintext TX tests cover exact and
excess inline/source output, multiple submissions and processing turns.

<a name="http11"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-http11.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-http11.svg" alt="HTTP/1.1">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-http11-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-http11-dark.svg" alt="HTTP/1.1">
  </a>
</h2>

`unit/protocol/http/v11/policy_behavior_tests.cpp` feeds whole requests and
one-byte fragments with a following sentinel request. Rejection must prevent
both handler invocation and sentinel dispatch, and stay terminal on later
input/turns. Accepted controls must return both replies in order.

| Field | Behavioral checks |
| --- | --- |
| `max_request_body_size` | Raw/chunked 0, finite boundaries and cumulative chunks through the engine; default 16 MiB boundary in `decoder_tests.cpp`. |
| `max_forwarding_hops` | Custom/zero limits; combined Via, Forwarded and X-Forwarded-For, including repeated fields. |
| `max_transfer_codings` | Custom/zero limits, one list versus repeated fields and final chunked coding. |
| `allow_chunked` | Both switch values, ordinary Content-Length positive control and terminal rejection. |
| `allow_upgrade` | Both switch values, valid Upgrade offer and ordinary-request positive control. Acceptance stays on HTTP/1.1. |
| `max_pending_requests` | Exact credit and resumed admission in `policy_budget_audit_tests.cpp`; windows 1/2/4 combined with turn and storage policies. |
| `max_pending_response_bytes` | Exact serialized response boundary and queued reader headers; overflow cancels retained frames in `operating_limits_tests.cpp` and `policy_budget_audit_tests.cpp`. Combined-policy pipelines retain the configured byte budget. |
| `max_task_steps_per_turn` | Zero rejected; exact start/resume bounds and sibling progress in `engine_concurrency_tests.cpp`; budgets 1/3 through 24-request pipelines in the combined-policy test. |
| `max_deferred_input_size` | Zero rejected, exact boundary and byte-by-byte accumulation; repeated drain/refill and terminal overflow. |
| `max_query_parameters` | Custom/zero limits, repeated pairs and empty separators; expanded/default limits in `decoder_tests.cpp` and native `server_limits_tests.cpp`. |
| `max_chunk_extension_size` | Custom/zero limits and exact extension boundary with terminal sentinel check; native default boundaries. |
| `max_trailer_section_size` | Custom/zero limits and exact wire boundary with terminal sentinel check; native default boundaries. |
| `max_request_head_size` | Zero rejected, exact complete/incomplete limits in `operating_limits_tests.cpp`, configured limits in `decoder_tests.cpp` and native default boundary. |
| `request_body_memory_threshold` | Zero and size−1/size/size+1 with real temporary-file observation before completion, exact raw/chunked payload and cleanup in `decoder_tests.cpp`. Chunked storage counts received framing bytes too. |
| `max_response_head_size` | Zero rejected, custom constructor capacities, deferred Content-Length boundary, failed growth preserving previous headers/body in `response_tests.cpp`; engine propagation in `engine_tests.cpp`. |
| `response_body_memory_threshold` | Zero and below/at/above threshold: actual inline/source choice and binary output; combined pending/turn/source policies. |

The combined-policy test checks all 18 combinations of three request windows,
two turn budgets and three inline thresholds. Each runs 24 yielded requests,
holds sources, advances work while output is busy, drains it, and compares the
complete response stream to an independently constructed expected stream.
Another case sets every numeric HTTP policy to SIZE_MAX and processes a small
valid chunked pipeline, checking for wraparound or eager allocation of a limit.

<a name="cache"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-cache.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-cache.svg" alt="Cache">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-cache-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-cache-dark.svg" alt="Cache">
  </a>
</h2>

The shared file cache also has policies. `unit/common/cache_tests.cpp` covers
each field: `max_entries` evicts the LRU, `max_cost` bounds resident cost while
responses retain ownership, and `max_loads` bounds concurrent loaders even
after invalidation. Each zero value disables caching without invoking a loader.
Failed loads release capacity and permit retry.

<a name="execution-and-scope"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-execution-and-scope.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-execution-and-scope.svg" alt="Execution and scope">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-execution-and-scope-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-execution-and-scope-dark.svg" alt="Execution and scope">
  </a>
</h2>

The new HTTP and TCP files have explicit CTest groups:
`doba_http_policy_tests` and `doba_transport_policy_tests`. CI inventory and
qualification campaigns require both groups; sanitizer campaigns run them too.
The complete unit suite also includes the decoder storage checks. TLS checks
remain in the existing TLS groups, requiring a TLS-enabled build.

These are bounded correctness and overload checks. They do not establish an
RSS ceiling, an idle/handshake timeout, a maximum shutdown duration or stability
under hours of sustained load. Those are separate contracts or campaigns.

<a name="local-validation-2026-11-11"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-local-validation-2026-11-11.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-local-validation-2026-11-11.svg" alt="Local validation, 2026-11-11">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-local-validation-2026-11-11-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-local-validation-2026-11-11-dark.svg" alt="Local validation, 2026-11-11">
  </a>
</h2>

- Complete Release suites with TLS and examples: Linux Clang 22/22 CTest
  groups; Windows MSVC 24/24. The final SIZE_MAX HTTP case also passes the
  focused Windows policy group.
- Linux ASan/UBSan: all eleven HTTP policy cases, all 204 decoder cases,
  all nine TCP policy cases and all 114 TLS processing invocations passed.
  Leak detection and halt-on-error were enabled.
- CI inventory checks passed on both platforms. One-round TLS-enabled
  qualification campaigns passed on each platform too.

Logs and campaign manifests are under `out/policy-*`; these are local runs,
not a new remote CI result or a whole-suite sanitizer run.
