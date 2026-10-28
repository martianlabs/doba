<a id="tcp-policies"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_policies/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_policies/title.svg" alt="tcp_policies">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_policies/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_policies/title-dark.svg" alt="tcp_policies">
</a>

**Header:** [include/transport/server/tcp_policies.h](../../../../include/transport/server/tcp_policies.h)

TCP operating policies.

The new fields `max_connections`, `inactivity_period` and `max_pending_sources`
are declared after `max_buffered_send_bytes` and before `ip` and `port`.
C++ designated initializers must follow that declaration order.

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server

[tcp_policies](#type-tcp-policies)

### <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> transport::server::tcp_policies

[kDefaultReceiveBufferCapacity](#constant-kdefaultreceivebuffercapacity) ·
[receive_buffer_capacity](#field-receive-buffer-capacity) ·
[worker_count](#field-worker-count) ·
[listen_backlog](#field-listen-backlog) ·
[max_buffered_send_bytes](#field-max-buffered-send-bytes) ·
[ip](#field-ip) ·
[port](#field-port)

[max_connections](#field-max-connections)

[inactivity_period](#field-inactivity-period)

[max_pending_sources](#field-max-pending-sources)

<a id="api"></a>
<a href="../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-tcp-policies"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tcp_policies

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct tcp_policies { ... };
```

TCP operating policies.

**Source:** [include/transport/server/tcp_policies.h:40](../../../../include/transport/server/tcp_policies.h#L40)

<a id="constant-kdefaultreceivebuffercapacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultReceiveBufferCapacity

**CONSTANT** · *transport::server::tcp_policies*

```cpp
static constexpr std::size_t kDefaultReceiveBufferCapacity = 4 * 1024;
```

Default receive buffer capacity, in bytes.

**Value:** `4 * 1024`

**Source:** [include/transport/server/tcp_policies.h:42](../../../../include/transport/server/tcp_policies.h#L42)

<a id="field-receive-buffer-capacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> receive_buffer_capacity

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t receive_buffer_capacity = kDefaultReceiveBufferCapacity;
```

Receive I/O block size per connection, in bytes (must be positive). Engines receive only fresh bytes and own incomplete protocol messages. This capacity is independent of protocol head and body limits.

**Value:** `kDefaultReceiveBufferCapacity`

**Source:** [include/transport/server/tcp_policies.h:46](../../../../include/transport/server/tcp_policies.h#L46)

<a id="field-worker-count"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> worker_count

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t worker_count = 0;
```

Number of workers (0 selects automatically).

**Value:** `0`

**Source:** [include/transport/server/tcp_policies.h:48](../../../../include/transport/server/tcp_policies.h#L48)

<a id="field-listen-backlog"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> listen_backlog

**FIELD** · *transport::server::tcp_policies*

```cpp
int listen_backlog = 0;
```

Requested pending connection queue size (0 uses the system default).

**Value:** `0`

**Source:** [include/transport/server/tcp_policies.h:50](../../../../include/transport/server/tcp_policies.h#L50)

<a id="field-max-buffered-send-bytes"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_buffered_send_bytes

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t max_buffered_send_bytes = 1024 * 1024;
```

Queued send reservations plus the entire active batch, per connection. Also bounds each accumulated batch; source-owned storage is excluded. Must be positive; exceeding it closes the connection. No preallocation. Partial sends retain the entire batch reservation until its final byte is consumed; this is a logical byte budget, not a bound on allocator capacity or process memory.

**Value:** `1024 * 1024`

**Source:** [include/transport/server/tcp_policies.h:54](../../../../include/transport/server/tcp_policies.h#L54)

<a id="field-ip"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ip

**FIELD** · *transport::server::tcp_policies*

```cpp
std::string ip;
```

IPv4 bind address and port (1-65535); both must be set.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tcp_policies.h:63](../../../../include/transport/server/tcp_policies.h#L63)

<a id="field-port"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> port

**FIELD** · *transport::server::tcp_policies*

```cpp
std::string port;
```

Stores the port value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tcp_policies.h:64](../../../../include/transport/server/tcp_policies.h#L64)

<a id="field-max-connections"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_connections

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t max_connections = 65535;
```

Maximum live context reservations per transport instance, shared by all workers. Zero means unlimited. Reserve before constructing protocol or TLS state; excess sockets close without an HTTP response. A reservation survives socket closure until accepted asynchronous work has completed and its context is retired. No contexts are preallocated.

**Source:** [include/transport/server/tcp_policies.h:57](../../../../include/transport/server/tcp_policies.h#L57)

<a id="field-inactivity-period"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> inactivity_period

**FIELD** · *transport::server::tcp_policies*

```cpp
std::chrono::milliseconds inactivity_period = std::chrono::seconds{60};
```

Sampled period without positive socket I/O, including TLS negotiation and graceful close. Zero disables expiration; negative periods are rejected during transport construction (the first `start()` for the public HTTP/HTTPS server). Queueing work does not renew the period. Recording a context's activity needs no clock read; workers query time for maintenance scheduling and sampling. They sample activity every min(period, 250 ms); the first sample starts the clock and a sample observing activity renews it. Expiration is never earlier than the configured period and normally occurs within two additional sample intervals (at most 500 ms), plus scheduling and scan delay. It aborts the socket without forcibly cancelling handlers. This is not an absolute request deadline or a minimum transfer rate.

**Source:** [include/transport/server/tcp_policies.h:59](../../../../include/transport/server/tcp_policies.h#L59)

<a id="field-max-pending-sources"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_pending_sources

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t max_pending_sources = 32;
```

Maximum retained output readers per connection, including the active reader. Zero means unlimited. Sources with empty prefixes still count. Credit is released on retirement or destruction, including errors and connection closure. Source-owned memory and files are outside the inline byte budget.

**Source:** [include/transport/server/tcp_policies.h:61](../../../../include/transport/server/tcp_policies.h#L61)
