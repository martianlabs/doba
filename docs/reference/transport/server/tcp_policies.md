<a id="tcp-policies"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_policies/index.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_policies/index.svg" alt="tcp_policies">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_policies/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_policies/index-dark.svg" alt="tcp_policies">
</a>

**Header:** [include/transport/server/tcp_policies.h](../../../../include/transport/server/tcp_policies.h)

TCP operating policies.

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

<a id="type-tcp-policies"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> tcp_policies

**TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct tcp_policies { ... };
```

TCP operating policies.

**Source:** [include/transport/server/tcp_policies.h:39](../../../../include/transport/server/tcp_policies.h#L39)

<a id="constant-kdefaultreceivebuffercapacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultReceiveBufferCapacity

**CONSTANT** · *transport::server::tcp_policies*

```cpp
static constexpr std::size_t kDefaultReceiveBufferCapacity = 4 * 1024;
```

Default receive buffer capacity, in bytes.

**Value:** `4 * 1024`

**Source:** [include/transport/server/tcp_policies.h:41](../../../../include/transport/server/tcp_policies.h#L41)

<a id="field-receive-buffer-capacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> receive_buffer_capacity

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t receive_buffer_capacity = kDefaultReceiveBufferCapacity;
```

Receive I/O block size per connection, in bytes (must be positive). Engines receive only fresh bytes and own incomplete protocol messages. This capacity is independent of protocol head and body limits.

**Value:** `kDefaultReceiveBufferCapacity`

**Source:** [include/transport/server/tcp_policies.h:45](../../../../include/transport/server/tcp_policies.h#L45)

<a id="field-worker-count"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> worker_count

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t worker_count = 0;
```

Number of workers (0 selects automatically).

**Value:** `0`

**Source:** [include/transport/server/tcp_policies.h:47](../../../../include/transport/server/tcp_policies.h#L47)

<a id="field-listen-backlog"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> listen_backlog

**FIELD** · *transport::server::tcp_policies*

```cpp
int listen_backlog = 0;
```

Requested pending connection queue size (0 uses the system default).

**Value:** `0`

**Source:** [include/transport/server/tcp_policies.h:49](../../../../include/transport/server/tcp_policies.h#L49)

<a id="field-max-buffered-send-bytes"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_buffered_send_bytes

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t max_buffered_send_bytes = 1024 * 1024;
```

Queued send reservations plus the entire active batch, per connection. Also bounds each accumulated batch; source-owned storage is excluded. Must be positive; exceeding it closes the connection. No preallocation.

**Value:** `1024 * 1024`

**Source:** [include/transport/server/tcp_policies.h:53](../../../../include/transport/server/tcp_policies.h#L53)

<a id="field-ip"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ip

**FIELD** · *transport::server::tcp_policies*

```cpp
std::string ip;
```

IPv4 bind address and port (1-65535); both must be set.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tcp_policies.h:55](../../../../include/transport/server/tcp_policies.h#L55)

<a id="field-port"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> port

**FIELD** · *transport::server::tcp_policies*

```cpp
std::string port;
```

Stores the port value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tcp_policies.h:56](../../../../include/transport/server/tcp_policies.h#L56)
