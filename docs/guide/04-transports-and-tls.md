<a name="transports-and-tls"></a>
<h1>
  <a href="../../resources/docs/guide/h1-transports-and-tls.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h1-transports-and-tls.svg" alt="Transports and TLS">
  </a>
  <a href="../../resources/docs/guide/h1-transports-and-tls-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h1-transports-and-tls-dark.svg" alt="Transports and TLS">
  </a>
</h1>

Your HTTP handlers deal with requests and responses. The transport deals with
the socket underneath them. Use *http_server* for TCP and *https_server* for
optional TLS on Linux or Windows.


Both backends deliver RX bytes to one engine per connection and copy its
inline output before send returns. Input is serialized; the engine invokes
send/close only within a serialized input or processing turn. It retains any
incomplete protocol message after the call. No callback can escape for later
or cross-thread use. A blocking engine or reader occupies its I/O worker.

Linux uses independent SO_REUSEPORT listeners and epoll workers. Windows uses
IOCP, AcceptEx, WSARecv and WSASend, allowing kernel RX/TX overlap while engine
calls stay serialized. Both support inline pipelining and one terminal reader.
Application scheduling, inactivity timers and pressure/resume integration
are outside the current transport scope. The
[TCP contract](../reference/transport/server/tcp.md#runtime-contract) defines
the full boundary and the differences in admission and stop behavior.

<a name="tcp-by-default"></a>
<h2>
  <a href="../../resources/docs/guide/h2-tcp-by-default.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-tcp-by-default.svg" alt="TCP by default">
  </a>
  <a href="../../resources/docs/guide/h2-tcp-by-default-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-tcp-by-default-dark.svg" alt="TCP by default">
  </a>
</h2>

**http_server<>** uses the TCP transport unless you choose another one. The
[hello_world example](../../examples/http/v11/hello_world/README.md) gives
it an IPv4 address and port, then starts the listener. Here is the relevant
bit of [main.cpp](../../examples/http/v11/hello_world/main.cpp):

```cpp
http_server<> http_server({.ip = "0.0.0.0", .port = "8080"});
http_server.add_route("GET", "/hello", [](const request&, response& res) {
  res.ok_200();
  res.add_header("Server", "doba.")
      .add_header("Content-Type", "text/plain; charset=utf-8")
      .set_body("ok");
});
http_server.start();
```

**0.0.0.0** listens on all IPv4 interfaces; use **127.0.0.1** when the
server should accept local connections only. TCP moves bytes between each
connection and its protocol engine. On Windows, its workers use IOCP; on
Linux, they use epoll. Your handlers do not need a platform-specific path.

<a name="tcp-policies"></a>
<h2>
  <a href="../../resources/docs/guide/h2-tcp-policies.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-tcp-policies.svg" alt="TCP policies">
  </a>
  <a href="../../resources/docs/guide/h2-tcp-policies-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-tcp-policies-dark.svg" alt="TCP policies">
  </a>
</h2>

The first **http_server<>** constructor argument is a *tcp_policies* value. It
sets up the listener and its workers. The
[policy definition](../../include/transport/server/tcp_policies.h) has the
full list. The [policy reference](../reference/transport/server/tcp_policies.md)
records platform ranges and coverage; the common settings are:

- **Address.** *ip* is the IPv4 bind address, and *port* is a string holding
  a port from **1** to **65535**. Both are complete numeric strings: no hostnames,
  NUL bytes or trailing data. Set both before starting the server.
- **Workers.** On Windows, **0** selects from hardware concurrency. The
  Linux backend uses one worker for **0**, or the explicit positive count.
- **Queue.** *listen_backlog* requests the pending connection queue size.
  **0** passes SOMAXCONN to the kernel, which can cap the queue.
- **Buffers.** *receive_buffer_capacity* reserves a fixed receive buffer per
  connection. It controls the I/O block size, independently of HTTP head and
  body limits. Each callback receives only fresh bytes; TCP reuses the full
  buffer before the next allowed receive. On Linux,
  *max_buffered_send_bytes* bounds TX without preallocating it. Windows also
  bounds its active socket batch plus accumulated inline bytes.
  Both must be positive. TX counts logical bytes, excluding allocator capacity
  and reader backing storage.
- **Admission.** *max_connections* defaults to **65535** across the server's
  workers. **0** removes the limit; excess sockets close before engine creation.
  Linux partitions a finite limit across workers without a shared counter.
  It must cover every effective worker. A full worker rejects even if another
  has unused capacity. Windows shares the budget across workers and retains a
  slot until pending I/O and the context's final owner release it.
- **Readers.** Both backends accept one terminal reader. There is no reader
  queue or configurable reader count. Later nonempty output or a second reader
  closes the connection; new input waits until it drains. A non-null empty
  reader is still terminal. Empty sends without a reader are no-ops, including
  after a reader. A pipeline batch cannot require multiple pending readers.

You can pass a named policy value when a designated initializer starts to
feel crowded:

```cpp
martianlabs::doba::transport::server::tcp_policies configuration;
configuration.ip = "127.0.0.1";
configuration.port = "8080";
configuration.worker_count = 4;
martianlabs::doba::http_server<> http_server(configuration);
```

Keep the default buffer sizes until you have a concrete reason to change
them; the [limits chapter](06-limits-and-operation.md#bound-work-in-flight)
covers resource boundaries, backpressure and zero values in more detail.
There is no transport pressure/resume handshake; send limits are hard caps.

<a name="switch-on-tls"></a>
<h2>
  <a href="../../resources/docs/guide/h2-switch-on-tls.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-switch-on-tls.svg" alt="TLS">
  </a>
  <a href="../../resources/docs/guide/h2-switch-on-tls-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-switch-on-tls-dark.svg" alt="TLS">
  </a>
</h2>

Configure `DOBA_ENABLE_TLS=ON` with OpenSSL 3.0 or newer and link
`martianlabs::doba_tls`. Installed consumers request the `TLS` component.
`https_server<>` accepts `tls_policies`: set the bind address, port, PEM chain
and matching unencrypted key. HTTPS selects `http/1.1` automatically and rejects
other ALPN tokens; the generic TLS transport remains protocol-independent.

TLS owns encryption/decryption buffers and reuses the unchanged TCP transport.
It supports TLS 1.2/1.3, synchronous/cooperative handlers and bounded
prefix/source output.
A source is read completely during the send callback and counts toward the
plaintext limit; this version does not stream arbitrarily large files.
See the [complete contract](../reference/transport/server/tls.md),
[policies](../reference/transport/server/tls_policies.md),
[HTTPS example](../../examples/http/v11/https_hello_world/README.md) and
[custom TLS protocol](../../examples/protocol/tls_echo/README.md).

Configuration and start/stop belong to a sequential external owner. Engine close
drains accepted valid output. Linux administrative stop cancels pending TX after
the active call returns; Windows drains accepted TX and real I/O completions.
Neither backend waits for application tasks or supplies a shutdown deadline.
See the [lifecycle contract](../reference/transport/server/tcp.md#runtime-contract).

The shared [processing contract](../reference/protocol/processing_result.md)
defines both returned results. Linux and Windows TCP deliver native turns;
TLS forwards them through the same serialized connection entries. The engine
owns coroutine frames and response ordering. Ciphertext still in flight does
not prevent a due handler from advancing. TLS control records preserve the
engine's outstanding scheduling snapshot.
