# TLS validation coverage

This inventory covers the current TCP-based TLS class and cooperative engine
turns on Linux and Windows. It is a functional contract matrix, not a claim of
100% line or branch coverage. External application callbacks and retired
transport mechanisms are excluded. TCP and common::reader retain their contracts.

| Supported contract | Direct evidence |
| --- | --- |
| TLS 1.2/1.3 and certificate/hostname verification | [wire tests](../../../../tests/integration/tls/exercise.py), [OpenSSL peer](../../../../tests/integration/tls/core.cpp) |
| Fragmented records, RX capacity, multi-record input | one-byte ciphertext writes, 33 KiB input, core RX capacity seven bytes |
| Bounded BIO progress and KeyUpdate | core BIO capacities 1/127/17408 in both versions; TLS 1.3 KeyUpdate |
| Prefix/body/source order, empty sends and empty source | wire source, empty-source and empty-source/close cases |
| Terminal source and immediate source release | later output, second source and shared-owner release checks |
| Inline/source and cumulative plaintext budgets | exact 4096-byte output, one-byte overflow and cumulative sends |
| Ciphertext budget including control records | handshake failure at one byte; large output fails without exposing its prefix; healthy connection recovers |
| Configuration fields and rejected credentials | [configuration executable](../../../../tests/integration/tls/configuration.cpp): zero/oversized capacities, NUL paths, 255/256-byte ALPN, missing/malformed/mismatched/encrypted keys |
| Generic ALPN and HTTP-only HTTPS ALPN | configured token, mismatches, HTTPS auto-selection, rejection of h2 configuration and clients without ALPN |
| No renegotiation, mTLS, early data or session resumption | configuration options and protocol-bound getters; these are excluded capabilities, not advertised optional implementations |
| Engine/peer close, failures and independent sessions | wire close_notify, failed reader, invalid input, EOF, concurrent sessions and recovery |
| Partial send/progress and pending-output lifetime | large bounded source to a slow peer, continuation and peer teardown |
| Admission during TLS negotiation and restart | one held connection at the admission limit; rejection/recovery; request before and after explicit restart |
| Processing snapshot, control-only RX and output-idle | [processing peer](../../../../tests/integration/tls/processing.cpp): all interest combinations, retained KeyUpdate deadline, busy ciphertext, close precedence |
| Prefix/source and failures from processing turns | exact/overflow/cumulative plain budgets, encoded budget, failed reader, second reader, handler exception after encoding; 90 cases across two TLS versions and three BIO capacities |
| Concurrent ordered HTTPS, cancellation and stop | [native processing tests](../../../../tests/integration/tls/processing.py): later sync request releases a waiting predecessor on the same connection; mixed reader output, failure, close, separate-connection progress, timed-frame cancellation and stop |
| HTTPS constructor with authentication providers | authenticated tuple construction; 401 rejection and authorized 200 response over TLS |
| Optional compilation and dependency isolation | [negative header consumer](../../../../tests/package/tls_guard/CMakeLists.txt), package consumers; CI builds plain consumer from TLS installation with OpenSSL discovery disabled |
| HTTPS and non-HTTP usage examples | [example runner](../../../../tests/integration/tls/examples.py) executes both built examples with verified clients, validates output and the echo line limit |

The [test inventory](../../../../tests/integration/tls/README.md) lists CTest
entry points and prerequisites. The [public TLS contract](tls.md) and
[policy table](tls_policies.md) define supported behavior and defaults.

No handshake/inactivity/shutdown timeout, dynamic SNI/certificate reload,
arbitrarily large source streaming, transport executor, pressure/resume hooks
or authenticated raw TCP EOF event is provided. These exclusions are explicit
limits; historical tests do not stand in for implementations of these features.

The standalone benchmark driver remains TCP-only. Published benchmark campaigns
are versioned measurements, not API documentation or qualification of this TLS
class. Required-check names retained for repository protection are identifiers;
the dedicated TLS jobs execute the current tests.
