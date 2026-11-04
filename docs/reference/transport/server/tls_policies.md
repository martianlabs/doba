<a id="tls-policies"></a>
<a href="../../../../resources/docs/reference/transport/server/tls_policies/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tls_policies/title.svg" alt="tls_policies">
</a>
<a href="../../../../resources/docs/reference/transport/server/tls_policies/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tls_policies/title-dark.svg" alt="tls_policies">
</a>

**Header:** [include/transport/server/tls_policies.h](../../../../include/transport/server/tls_policies.h)

`tls_policies` inherits `tcp_policies`. All admission, worker and binding
settings retain their TCP meaning. TLS uses these buffer settings:

| Property | Default | Meaning |
| --- | --- | --- |
| `receive_buffer_capacity` | 4 KiB | Decrypted block borrowed by the engine. Positive, at most `INT_MAX`. |
| `max_buffered_send_bytes` | 1 MiB | Plaintext produced per TCP RX or processing batch, including prefix, body and all source bytes. Positive. |
| `encrypted_receive_buffer_capacity` | 17 KiB | TCP socket receive block size. Positive; subject to native TCP limits. |
| `network_bio_buffer_capacity` | 17 KiB | Capacity of each direction of the BIO pair. Positive and at most `INT_MAX`. |
| `max_buffered_ciphertext_bytes` | 2 MiB | Encoded batch limit, including handshake, record overhead and close alerts; also passed to TCP as its TX budget. Positive; subject to native TCP limits. |
| `certificate_file` | empty | PEM certificate chain path; required. |
| `private_key_file` | empty | Matching unencrypted PEM private key path; required. |
| `alpn_protocol` | empty | One optional protocol token, at most 255 bytes. Empty disables ALPN. |

The two output limits are independent. The plaintext budget starts at each
TCP RX or processing call. Ciphertext includes every record generated in that
call, including control records and close alerts.
Existing TCP also enforces its encoded budget across output still in flight;
TLS does not bypass that limit. Handshake output can exceed the encoded limit
without any application output. No partial current batch is submitted on error.

These are logical byte limits, not an RSS bound: string capacity, the original
source's storage and OpenSSL's internal allocations are separate. Sources are
consumed synchronously, not streamed under TCP backpressure. Use a suitable
finite plaintext budget for the largest supported source; exceeding it closes
the connection.

Setting `alpn_protocol` to `http/1.1` is appropriate for HTTPS. A client offering
only different tokens is rejected; clients without ALPN remain permitted. TLS
itself does not assume an application protocol.
