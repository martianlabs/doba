<a id="tls"></a>
<a href="../../../../resources/docs/reference/transport/server/tls/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tls/title.svg" alt="tls">
</a>
<a href="../../../../resources/docs/reference/transport/server/tls/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tls/title-dark.svg" alt="tls">
</a>

**Header:** [include/transport/server/tls.h](../../../../include/transport/server/tls.h)

`tls<Engine, Factory>` is a concrete TCP-derived transport on Linux and
Windows. TCP hosts a serialized TLS engine adapter; its platform backends,
contexts, buffers and synchronization are unchanged. Plain TCP neither includes
OpenSSL nor checks whether TLS is enabled.

Build with `DOBA_ENABLE_TLS=ON`, link `martianlabs::doba_tls`, and configure a
PEM certificate chain and matching unencrypted PEM private key. Installed
consumers use `find_package(doba CONFIG REQUIRED COMPONENTS TLS)`. A consumer
requesting only `martianlabs::doba` does not discover or link OpenSSL, even when
the installation also includes TLS. OpenSSL 3.0 or newer is required; use a
maintained vendor build or a supported upstream release such as 3.5 LTS.
On Windows, provide an OpenSSL root or a suitable CMake toolchain, and make its
SSL/crypto runtime DLLs available to the executable. These dependencies belong
to the TLS target; they are not installed or required by plain TCP.

The factory creates one protocol engine and one TLS session per accepted TCP
connection. An immutable shared owner keeps credentials alive until the last
session is destroyed. No extra thread, executor, mutex or socket I/O is added.
Connection notifications describe TCP admission, not successful TLS negotiation.

**Runtime contract**

- TLS 1.2 and 1.3 are supported. TLS processing, engine input, processing turns and output are
  serialized by the existing TCP connection owner. The engine receives only
  decrypted application bytes, borrowed until its input call returns.
- Both engine entry points return the shared processing_result. Immediate,
  absolute-deadline and drained-output interests pass through TLS unchanged;
  control-only records preserve the previous snapshot. Close clears scheduling.
- Send and close callbacks are valid only inside an engine input or processing
  call, on that call's thread. Coroutine frames and cooperative resumption
  belong to the engine; TLS never receives handles or external completions.
  Output-idle reflects all previously accepted encrypted TCP output. Busy
  ciphertext does not prevent a deadline turn from progressing handlers.
- `prefix`, `body` and one terminal `source` are consumed in order. A source is
  read synchronously to completion; all its bytes count toward the finite
  plaintext batch budget. It must not block indefinitely. An additional source
  or later nonempty send in the same engine call fails the connection. Empty
  sends without a source remain no-ops before close.
- This version does not stream arbitrarily large sources. It accumulates a
  finite ciphertext batch and calls TCP only after processing succeeds. A failed
  source, source without progress, or exceeded limit discards the current batch
  and closes the connection. Output from earlier successful batches may already
  have reached the peer; this is not a transaction across multiple input/processing calls.
- Fully consumed source ownership is released before the send callback returns.
  More decrypted input may then reach the engine within the same TCP batch.
- A plaintext output block groups prefix/body/source bytes into records of at
  most 16 KiB. Pipelined responses do not force a TLS record per callback.
- Two bounded BIO buffers carry records between the adapter and OpenSSL. The
  adapter drains output while retrying `WANT_WRITE` with stable write arguments;
  handshake and receive `WANT_READ` wait for the next TCP input. Application
  writes do not suspend for peer input; an unexpected `WANT_READ` there fails
  the connection. Renegotiation and post-handshake authentication are disabled.
  TLS 1.3 KeyUpdate is supported and tested.
- Engine close and peer `close_notify` generate our `close_notify`, then ask TCP
  to drain and close. The adapter does not wait for the peer's shutdown reply.
  Fatal TLS errors discard the current batch and close without `SSL_shutdown`.
  A raw TCP EOF simply destroys the session: it is not delivered to the engine
  as an authenticated TLS closure or an application message terminator.
- Administrative start/stop, admission, partial socket writes and platform
  shutdown behavior remain TCP's responsibility. Administrative stop is not a
  TLS graceful shutdown handshake. There is no handshake or shutdown timer;
  stalled handshakes consume the configured connection admission allowance.
- One fixed chain/key and optional single ALPN token are configured before
  listening. Missing credentials, mismatched keys and invalid limits throw
  during transport construction, before listening. The HTTP server constructs
  its transport on first start. Encrypted keys requiring a password are rejected without an
  interactive prompt. No mTLS, dynamic SNI certificates, certificate reload,
  early data or session resumption is provided.

See [TLS policies](tls_policies.md), the
[TCP contract](tcp.md#runtime-contract), and the
[HTTPS example](../../../../examples/http/v11/https_hello_world/README.md).

OpenSSL defines [BIO pair buffering](https://docs.openssl.org/3.5/man3/BIO_s_bio/)
and [retry/error requirements](https://docs.openssl.org/3.5/man3/SSL_get_error/).

[Validation coverage](tls-validation.md) maps the supported contract to tests
and executable examples. Internal implementation notes describe
[tls_engine](tls_engine.md) and [tls_configuration](tls_configuration.md);
these detail types are not supported customization APIs.
