# TLS integration

Enable `DOBA_ENABLE_TLS=ON`. Production consumers and independent native test
clients need OpenSSL 3.0 or newer. No interpreter or OpenSSL executable is
required by the TLS test groups.

| CTest group | Responsibility |
| --- | --- |
| `doba_tls_core_tests` | In-memory OpenSSL peer, verified TLS 1.2/1.3, seven-byte plaintext RX, bounded BIO progress, KeyUpdate and stable configuration ownership. |
| `doba_tls_tests` | Native wire tests: fragments, pipeline, source ownership and budgets, failed readers, slow peers, shutdown, admission/recovery, restart, ALPN and authenticated HTTPS. |
| `doba_tls_processing_tests` | 90 in-memory cases: processing snapshots, KeyUpdate, busy TX, prefix/source, limits, failures, concurrent mixed HTTP and close; both TLS versions and BIO capacities 1/127/17408. |
| `doba_tls_processing_wire_tests` | Native HTTPS turns, gated same-connection concurrency, ordered mixed/source output, failures, disconnect and stop cancellation with one worker; TLS 1.2/1.3. |
| `doba_tls_configuration_tests` | Policies, invalid and mismatched credentials, encrypted-key rejection without prompting and excluded OpenSSL capabilities. Temporary files are removed individually. |
| `doba_tls_examples_tests` | Both actual TLS example binaries, including an async HTTPS route, exchange data with verified clients. Registered when examples are enabled. Windows uses a hidden private console for the signal-based examples. |

Every group has an external CTest deadline. Wire operations and child processes
also have bounded waits. Configuration checks require the intended exception or
diagnostic, so an unrelated listener failure cannot satisfy the rejection test.
Native checks throw or fail explicitly and remain active in Release.

The core executable now checks all eight processing-interest combinations for
both TLS versions and BIO capacities 1/127/17408 (48 invocations). Each corrupts
a real authenticated record and requires zero plaintext delivery and no revived
scheduling. Native wire coverage also holds incomplete handshakes while a
healthy connection progresses. This does not introduce a handshake timeout.

The processing wire parser has independent malformed-response negative controls
and strict bounded Content-Length framing. Native child runners reject captured
sanitizer reports even when their server is intentionally terminated. See the
[security audit](../../security-audit.md) for properties and assurance limits.

`cert.pem` and `key.pem` are public test-only self-signed credentials. Never use
the key for deployment. Clients verify the certificate and hostname. Generated
mismatched/encrypted keys belong to unique temporary directories; cleanup is not
recursive and never alters the source credentials.

See the [contract coverage matrix](../../../docs/reference/transport/tls-validation.md).
The tests do not claim a library timeout, mTLS, arbitrary source streaming,
session resumption or authenticated raw TCP EOF notification.
