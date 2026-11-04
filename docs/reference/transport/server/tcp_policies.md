<a id="tcp-policies"></a>
<a href="../../../../resources/docs/reference/transport/server/tcp_policies/title.svg#gh-light-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_policies/title.svg" alt="tcp_policies">
</a>
<a href="../../../../resources/docs/reference/transport/server/tcp_policies/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../resources/docs/reference/transport/server/tcp_policies/title-dark.svg" alt="tcp_policies">
</a>

**Header:** [include/transport/server/tcp_policies.h](../../../../include/transport/server/tcp_policies.h)

Policies are copied when a transport instance is constructed and remain fixed
for its lifetime. The constructor validates capacities, backlog and worker/
admission combinations. Start validates the complete numeric endpoint before
creating its listener. See the [TCP runtime contract](tcp.md#runtime-contract)
for callback ownership, terminal readers, errors and administrative stop.

The declaration order is RX, workers, backlog, TX, admission, IP and port.
C++ designated initializers must follow that order.

| Policy | Default | Linux | Windows |
|---|---:|---|---|
| receive_buffer_capacity | 4096 bytes | 1..SSIZE_MAX | 1..ULONG_MAX |
| worker_count | 0 | Zero selects one; positive selects that many | Zero selects hardware concurrency, at least one; explicit counts fit DWORD |
| listen_backlog | 0 | Nonnegative, zero requests SOMAXCONN; kernel may cap | Same |
| max_buffered_send_bytes | 1 MiB | 1..SSIZE_MAX logical TX bytes | Any positive size_t budget; kernel submissions are ULONG-sized |
| max_connections | 65535 | Zero unlimited; finite budget partitioned across workers | Zero unlimited; shared reservations across workers |
| ip | Empty | Complete numeric IPv4 text required at start | Same |
| port | Empty | Complete decimal text in 1..65535 required at start | Same |

Kernel queues, allocator capacity, protocol message limits and reader backing
storage are independent of these logical transport bounds. A successfully
constructed configuration does not preallocate its buffers or guarantee that
start or a later connection can obtain the requested resources.

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

```cpp
struct tcp_policies { ... };
```

[kDefaultReceiveBufferCapacity](#constant-kdefaultreceivebuffercapacity) ·
[receive_buffer_capacity](#field-receive-buffer-capacity) ·
[worker_count](#field-worker-count) ·
[listen_backlog](#field-listen-backlog) ·
[max_buffered_send_bytes](#field-max-buffered-send-bytes) ·
[max_connections](#field-max-connections) · [ip](#field-ip) · [port](#field-port)

<a id="constant-kdefaultreceivebuffercapacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultReceiveBufferCapacity

**CONSTANT** · *transport::server::tcp_policies*

```cpp
static constexpr std::size_t kDefaultReceiveBufferCapacity = 4 * 1024;
```

Default RX block capacity, in bytes. It does not limit protocol message size.

**Source:** [include/transport/server/tcp_policies.h:41](../../../../include/transport/server/tcp_policies.h#L41)

<a id="field-receive-buffer-capacity"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> receive_buffer_capacity

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t receive_buffer_capacity = kDefaultReceiveBufferCapacity;
```

Maximum bytes in one socket receive block. The engine receives only fresh
borrowed bytes and retains incomplete messages itself. Zero and values beyond
the platform receive API range are rejected before listening.

**Source:** [include/transport/server/tcp_policies.h:45](../../../../include/transport/server/tcp_policies.h#L45)

<a id="field-worker-count"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> worker_count

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t worker_count = 0;
```

Zero selects one worker on Linux and hardware concurrency, at least one,
on Windows. A positive value selects that many workers. Linux workers own
independent SO_REUSEPORT listeners and contexts; Windows shares an IOCP.
Different connection engines may execute concurrently. Resource exhaustion can
still make startup fail; representability does not promise unlimited threads.

**Source:** [include/transport/server/tcp_policies.h:48](../../../../include/transport/server/tcp_policies.h#L48)

<a id="field-listen-backlog"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> listen_backlog

**FIELD** · *transport::server::tcp_policies*

```cpp
int listen_backlog = 0;
```

A nonnegative request passed to listen. Zero passes SOMAXCONN; the kernel
may cap the queue according to its own limits. This does not cap established
contexts. Negative values are rejected. INT_MAX is a valid request, not a
promise that the kernel can queue that many connections.

**Source:** [include/transport/server/tcp_policies.h:51](../../../../include/transport/server/tcp_policies.h#L51)

<a id="field-max-buffered-send-bytes"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_buffered_send_bytes

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t max_buffered_send_bytes = 1024 * 1024;
```

Positive logical buffered TX bytes per connection. The entire active batch
remains charged during a partial send; Windows also counts inline bytes
accumulating behind that batch. Reader chunks are bounded by this budget.
Exceeding it closes the connection. Zero never means unlimited.

No storage is preallocated by this value. String allocator capacity, process
RSS and storage owned by a reader are outside this cap. The fixed one-reader
restriction is independent of the byte budget. Linux requires a representable
ssize_t budget; Windows divides larger transfers into ULONG-sized submissions.

**Source:** [include/transport/server/tcp_policies.h:57](../../../../include/transport/server/tcp_policies.h#L57)

<a id="field-max-connections"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_connections

**FIELD** · *transport::server::tcp_policies*

```cpp
std::size_t max_connections = 65535;
```

Zero disables admission limiting. Each transport instance has an independent
budget; engines are allocated only after admission succeeds.

Linux divides a finite budget by worker count, with one extra slot for the
first remainder workers. The finite budget must cover every effective worker.
A full worker can reject while another has unused capacity; there is no shared
counter or redistribution. A slot is released with that worker's context.

Windows shares reservations across workers. The budget may be smaller than the
worker count. A reservation remains held after socket closure while real I/O
or another context owner still retains the context. The value does not
preallocate contexts or bound the kernel's pending handshakes.

**Source:** [include/transport/server/tcp_policies.h:61](../../../../include/transport/server/tcp_policies.h#L61)

<a id="field-ip"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> ip

**FIELD** · *transport::server::tcp_policies*

```cpp
std::string ip;
```

Numeric IPv4 bind address, empty by default and required at start. Hostnames,
IPv6, trailing data and embedded or trailing NUL bytes are rejected. No name
resolution is performed. Binding can fail even for valid text if the address
is unavailable locally.

**Source:** [include/transport/server/tcp_policies.h:63](../../../../include/transport/server/tcp_policies.h#L63)

<a id="field-port"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> port

**FIELD** · *transport::server::tcp_policies*

```cpp
std::string port;
```

Complete unsigned decimal port text in [1, 65535], empty by default and
required at start. Signs, whitespace, trailing data, NUL bytes, overflow and
port zero are rejected. Leading decimal zeros are accepted. A valid port may
still fail to bind because of availability or operating-system permissions.

**Source:** [include/transport/server/tcp_policies.h:65](../../../../include/transport/server/tcp_policies.h#L65)

<a id="validation-coverage"></a>

## <a href="../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> Validation and coverage

| Obligation | Evidence |
|---|---|
| Defaults, zero meanings, representable RX/TX and backlog extremes, worker/admission combinations | [Constructor tests](../../../../tests/unit/transport/server/tcp_transport_tests.cpp) |
| Complete endpoint text including NUL rejection, copied policies, RX bounds, automatic workers, kernel backlog requests, unlimited and finite admission, exact TX and overflow recovery | [Portable policy tests](../../../../tests/integration/transport/server/tcp_policy_limits_tests.cpp) |
| Partial TX budgets, reader bounds and ownership | [TX state tests](../../../../tests/unit/transport/server/send_state_tests.cpp), [Linux context tests](../../../../tests/unit/transport/server/tcp_context_linux_tests.cpp) |
| Partitioned Linux admission and factory/EOF recovery | [Linux integration](../../../../tests/integration/transport/server/tcpip_linux_tests.cpp) |
| Shared Windows admission, factory failure and restart | [Windows integration](../../../../tests/integration/transport/server/operating_policies_windows_tests.cpp) |

Representable maxima are checked at construction without allocating gigantic
buffers or worker groups. Real socket tests use available loopback ports.
Backlog tests verify that accepted requests work with the kernel; they do not
assert an exact kernel queue length. Port 1 and 65535 are parser bounds reviewed
in both backends; tests reject 0 and 65536 without requiring privileged or
potentially occupied boundary ports.
