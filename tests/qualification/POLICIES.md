<a name="policy-checks"></a>
<h1>
  <a href="../../resources/tests/qualification/policies/h1-policy-checks.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h1-policy-checks.svg" alt="Do the limits hold?">
  </a>
  <a href="../../resources/tests/qualification/policies/h1-policy-checks-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h1-policy-checks-dark.svg" alt="Do the limits hold?">
  </a>
</h1>

A setting isn't tested just because we can read its default back. We need
to see it reject extra work, bound a buffer or choose the right storage.
Here's where each policy gets that check.

<a name="tcp"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-tcp.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-tcp.svg" alt="TCP">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-tcp-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-tcp-dark.svg" alt="TCP">
  </a>
</h2>

[Socket policy tests](../integration/transport/tcp_policy_limits_tests.cpp)
use real listeners. [Unit helpers](../unit/transport/operating_policies_tests.cpp)
check partial-send accounting and reader ownership.

| Field | What we check |
| --- | --- |
| `receive_buffer_capacity` | Three-byte blocks, binary echo, zero and native size bounds. |
| `worker_count` | Automatic selection, independent progress and one/four-worker overload. |
| `listen_backlog` | 0, 1 and INT_MAX work; negative values fail. The kernel may cap it. |
| `max_buffered_send_bytes` | Exact capacity, one extra byte, partial sends and reader blocks. |
| `max_connections` | Zero, full admission, rejected factories, freed slots and recovery. |
| `ip` | Valid loopback, malformed addresses and embedded/trailing NUL. |
| `port` | Valid ports, zero, overflow, signs, whitespace and NUL. |

Overload keeps sixteen peers connected and starts 32 extra clients together.
Admitted clients must still get their exact replies. We free half the slots
and refill them, for eight cycles with one/four workers. Linux's per-worker
partition may reject early while filling; a full server must reject every extra peer.

<a name="tls"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-tls.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-tls.svg" alt="TLS">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-tls-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-tls-dark.svg" alt="TLS">
  </a>
</h2>

[Configuration](../integration/tls/configuration.cpp),
[processing](../integration/tls/processing.cpp) and
[socket tests](../integration/tls/exercise.cpp) cover the extra TLS fields.

| Field | What we check |
| --- | --- |
| `certificate_file` | Verified peer, missing/invalid certificate, NUL and key mismatch. |
| `private_key_file` | Missing, malformed, mismatched and encrypted keys; no prompt. |
| `encrypted_receive_buffer_capacity` | Zero fails; one/seventeen-byte socket reads still work. |
| `network_bio_buffer_capacity` | Zero/overflow fail; capacities 1/127/17408 make progress. |
| `max_buffered_ciphertext_bytes` | Batch size minus one, exact and plus one, including TLS overhead. |
| `alpn_protocol` | Empty, 255/256-byte bounds, matching and wrong offers, HTTP compatibility. |

Both TLS versions get the ciphertext checks. Overflow must emit no partial
batch or revive processing. Native checks also try three-byte plaintext input
and exact/excess plaintext output. Encrypted RX and TX limits feed through to TCP.

<a name="http11"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-http11.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-http11.svg" alt="HTTP/1.1">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-http11-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-http11-dark.svg" alt="HTTP/1.1">
  </a>
</h2>

[Policy cases](../unit/protocol/http/v11/policy_behavior_tests.cpp) send each
request whole and byte by byte, with a sentinel request behind it. Rejection
must stop both handlers and stay terminal on later input and turns.

| Field | What we check |
| --- | --- |
| `max_request_body_size` | Raw/chunked boundaries, cumulative chunks and zero. |
| `max_forwarding_hops` | Combined/repeated Via, Forwarded and X-Forwarded-For; zero. |
| `max_transfer_codings` | Lists, repeated fields, final chunked coding and zero. |
| `allow_chunked` | Both settings, plus an ordinary Content-Length request. |
| `allow_upgrade` | Both settings; accepted offers still use HTTP/1.1. |
| `max_pending_requests` | Exact window, paused admission and resumed work. |
| `max_pending_response_bytes` | Multiple queued replies, reader headers, overflow and reusable space. |
| `max_task_steps_per_turn` | Zero fails; starts/resumes stay within budget and all siblings progress. |
| `max_deferred_input_size` | Fragment totals, exact capacity, overflow and repeated refill. |
| `max_query_parameters` | Pairs, empty separators, custom/default limits and zero. |
| `max_chunk_extension_size` | Exact extension boundary, zero and terminal rejection. |
| `max_trailer_section_size` | Exact wire-byte boundary, zero and terminal rejection. |
| `max_request_head_size` | Zero fails; complete/incomplete heads and native boundaries. |
| `request_body_memory_threshold` | Real spill files below/at/above the threshold, payload and cleanup. |
| `max_response_head_size` | Zero fails; custom capacity, framing and failed growth preserving data. |
| `response_body_memory_threshold` | Actual inline/reader choice and exact binary bytes. |

The [decoder](../unit/protocol/http/v11/decoder_tests.cpp),
[response](../unit/protocol/http/v11/response_tests.cpp),
[budget](../unit/protocol/http/v11/policy_budget_audit_tests.cpp) and
[native limit](../integration/protocol/http/v11/server_limits_tests.cpp) cases
fill in the storage and socket checks.

Eighteen combinations of request windows, turn budgets and inline thresholds
each run 24 yielded requests. Held readers must not break reply order or stop
other work. Another case sets all numeric policies to SIZE_MAX and processes
a small valid pipeline without wrapping or allocating the limit up front.

<a name="cache"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-cache.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-cache.svg" alt="Cache">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-cache-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-cache-dark.svg" alt="Cache">
  </a>
</h2>

[Cache tests](../unit/common/cache_tests.cpp) check `max_entries` eviction,
`max_cost` accounting and `max_loads` while loads are still running, even
after invalidation. Zero in any field disables caching without calling the
loader. Failed loads must release their slot and allow a retry.

<a name="execution-and-scope"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-execution-and-scope.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-execution-and-scope.svg" alt="Where these tests run">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-execution-and-scope-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-execution-and-scope-dark.svg" alt="Where these tests run">
  </a>
</h2>

`doba_http_policy_tests` and `doba_transport_policy_tests` are required by
CI inventory and sanitizer campaigns. Decoder spill checks run in the unit
suite; TLS checks run in the TLS-enabled groups.

These are bounded overload tests, not hours of sustained load. Byte budgets
don't cap process RSS, and these tests don't add idle/handshake or stop timeouts.

<a name="local-validation-2026-11-11"></a>
<h2>
  <a href="../../resources/tests/qualification/policies/h2-local-validation-2026-11-11.svg#gh-light-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-local-validation-2026-11-11.svg" alt="Local runs, 2026-11-11">
  </a>
  <a href="../../resources/tests/qualification/policies/h2-local-validation-2026-11-11-dark.svg#gh-dark-mode-only">
    <img src="../../resources/tests/qualification/policies/h2-local-validation-2026-11-11-dark.svg" alt="Local runs, 2026-11-11">
  </a>
</h2>

| Run | Result |
| --- | --- |
| Release, TLS and examples | Linux 22/22 groups; Windows 24/24. Final SIZE_MAX case also passed the Windows policy group. |
| Linux ASan/UBSan | 11 HTTP policy, 204 decoder, 9 TCP policy and 114 TLS processing cases passed. Leak checks enabled. |
| Inventory and one-round TLS campaigns | Both OSes passed; 77 actions on Linux, 61 on Windows. |

Logs are in `out/policy-*`. These are local results; the sanitizer runs cover
the listed tests, not the whole suite.
