<a name="limits-and-operation"></a>
<h1>
  <a href="../../resources/docs/guide/h1-limits-and-operation.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h1-limits-and-operation.svg" alt="Limits and operation">
  </a>
  <a href="../../resources/docs/guide/h1-limits-and-operation-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h1-limits-and-operation-dark.svg" alt="Limits and operation">
  </a>
</h1>

A busy server needs boundaries. doba gives you one set of knobs for the
transport and another for HTTP/1.1. Some reject work that is too large;
others decide how much stays in memory. Knowing which is which saves a lot
of head-scratching.

<a name="two-sets-of-policies"></a>
<h2>
  <a href="../../resources/docs/guide/h2-two-sets-of-policies.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-two-sets-of-policies.svg" alt="Two sets of policies">
  </a>
  <a href="../../resources/docs/guide/h2-two-sets-of-policies-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-two-sets-of-policies-dark.svg" alt="Two sets of policies">
  </a>
</h2>

The first **http_server<>** argument configures the transport. The second
configures the HTTP engine. The
[request_limits example](../../examples/http/v11/request_limits/README.md)
sets a five-byte body limit in its
[source](../../examples/http/v11/request_limits/main.cpp):

```cpp
policies limits;
limits.max_request_body_size = 5;
http_server<> http_server({.ip = "0.0.0.0", .port = "8080"}, limits);
```

The address belongs to TCP; *max_request_body_size* belongs to HTTP. You can
configure them independently. The [TCP policy][tcp-policy] and
[HTTP policy][http-policy] definitions list every setting. Chapter 4's
[TCP section][tcp-section] covers listener and worker choices.

[tcp-policy]: ../../include/transport/server/tcp_policies.h
[http-policy]: ../../include/protocol/http/v11/policies.h
[tcp-section]: 04-transports-and-tls.md#tcp-policies

<a name="limit-incoming-requests"></a>
<h2>
  <a href="../../resources/docs/guide/h2-limit-incoming-requests.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-limit-incoming-requests.svg" alt="Limit incoming requests">
  </a>
  <a href="../../resources/docs/guide/h2-limit-incoming-requests-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-limit-incoming-requests-dark.svg" alt="Limit incoming requests">
  </a>
</h2>

*max_request_body_size* defaults to **16 MiB**. It caps accepted request body
size, including chunked bodies. **0** removes this particular limit. The
**request_limits** example sets it to **5**; build and run that target from
the repository root:

```sh
cmake -S . -B out/build/guide
cmake --build out/build/guide --config Release --target request_limits
```

With a single-configuration generator on Linux, start:

```sh
./out/build/guide/examples/http/v11/request_limits/request_limits
```

With Visual Studio on Windows:

```powershell
$build = '.\out\build\guide\examples\http\v11\request_limits'
& "$build\Release\request_limits.exe"
```

In another terminal, try the boundary:

```sh
curl -i --data-binary "12345" http://localhost:8080/upload
curl -i --data-binary "123456" http://localhost:8080/upload
```

Five bytes get **200 OK** and **accepted**. Six get
**413 Content Too Large** before the route handler runs. Stop the example
with Ctrl+C afterward.

Other HTTP policies cap the request head, query parameter count, transfer
codings, chunk extensions, and trailers. They are independent of the body
size limit. Keep their defaults unless your application has a clear reason
to change them.

<a name="memory-and-large-bodies"></a>
<h2>
  <a href="../../resources/docs/guide/h2-memory-and-large-bodies.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-memory-and-large-bodies.svg" alt="Memory and large bodies">
  </a>
  <a href="../../resources/docs/guide/h2-memory-and-large-bodies-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-memory-and-large-bodies-dark.svg" alt="Memory and large bodies">
  </a>
</h2>

The decoder normally reads the transport buffer directly. Requests borrow
field bytes and inline bodies. Synchronous response bytes use the engine's
reusable output strings; asynchronous responses own their serialized bytes.
Storage starts empty; HTTP limits do not reserve memory up front.
*max_request_head_size* and *max_response_head_size* each default to
**4 KiB** and bound accepted and serialized heads, respectively.

The engine passes a view of the remaining input to
`deserialize(std::string_view&)` until it needs more bytes or closes.
Each successful call consumes one request. The input stays immutable and is
valid for the duration of the transport callback.

Only an incomplete message is copied into the decoder's residual vector.
On subsequent receives, the decoder adds just enough bytes to complete its
head; the body framer identifies the bytes belonging to that body. After
completion, remaining requests are read directly from the new transport
block. The residual vector is cleared for reuse, without reserving memory
in advance. The head parser still retries incomplete heads; body framing
continues incrementally.

Residual storage keeps the incomplete head and any inline body. Bodies above
the memory threshold use temporary storage, and their consumed wire bytes do
not remain in the residual vector. Transport capacity controls the I/O block
size independently of HTTP limits. Percent-encoded paths need a writable
copy of the path; other field bytes remain borrowed during synchronous work.

Before any asynchronous handler is invoked, the request copies its fields
and inline wire body into one owned vector. This includes handlers that
complete immediately. The engine does not copy bytes belonging to other
requests. The request and response then outlive the input and decoder
independently.

Each engine owns two initially empty strings for synchronous response heads
and inline bodies. They grow only when used and retain their capacity for
later responses; the policies do not preallocate their maximum sizes. The
response itself borrows these strings until the transport has preserved the
output in `on_send`. Replies queued behind pending asynchronous work are
copied into the existing output queue before the strings are reused. Decoder
interim and rejection replies own their storage in the returned result.
Asynchronous handlers keep independent response storage throughout their
execution.

An accepted body does not have to stay inline. The
*request_body_memory_threshold* defaults to **16 KiB**; a larger incoming
body moves to a temporary file. *max_request_body_size* still decides
whether the body is accepted.

The *response_body_memory_threshold* also defaults to **16 KiB**.
An inline body that exceeds it uses a writer with separate storage options.
Its *spill_to_disk_threshold* defaults to **128 KiB** and does not change
either HTTP memory threshold. The
[large_response_bodies example][large-bodies] uses a raw writer that spills
after **1024** bytes, then builds an **8192-byte** response. Its
[source](../../examples/http/v11/large_response_bodies/main.cpp) starts
the writer like this:

[large-bodies]: ../../examples/http/v11/large_response_bodies/README.md

```cpp
byte_storage_options options{.spill_to_disk_threshold = 1024, .spill_dir = {}};
auto writer = body::body_writer::raw(options);
```

When the writer passes that threshold, its storage moves to a temporary
file. The response API stays the same: the handler moves the finished writer
into *set_body*. To try it, build the example:

```sh
cmake --build out/build/guide --config Release --target large_response_bodies
```

On Linux with a single-configuration generator, run:

```sh
./out/build/guide/examples/http/v11/large_response_bodies/large_response_bodies
```

With Visual Studio on Windows:

```powershell
$build = '.\out\build\guide\examples\http\v11\large_response_bodies'
& "$build\Release\large_response_bodies.exe"
```

Then request **/large**:

```sh
curl --output large.bin http://localhost:8080/large
```

The download contains **8192 bytes**, all the letter **x**. Stop the server
before running another example on port **8080**.

<a name="bound-work-in-flight"></a>
<h2>
  <a href="../../resources/docs/guide/h2-bound-work-in-flight.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-bound-work-in-flight.svg" alt="Bound work in flight">
  </a>
  <a href="../../resources/docs/guide/h2-bound-work-in-flight-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-bound-work-in-flight-dark.svg" alt="Bound work in flight">
  </a>
</h2>

Body size is only one way a connection can use resources. Two more limits
are worth knowing:

- **Async responses.** Once a connection uses async handling,
  *max_pending_requests* caps response positions awaiting ordered delivery.
  It defaults to **128**. The bundled transports pause new request processing
  under pressure and resume it after credit is released. A hard overflow closes
  the connection. **0** removes this limit.
- **Queued output.** The TCP policy *max_buffered_send_bytes* defaults to
  **1 MiB** per connection. It bounds queued send reservations plus the
  active batch; partial sends retain the whole batch reservation until its
  last byte is consumed. A connection that exceeds it closes. Storage owned by a body
  source is outside this cap.

The transport also caps retained readers with *max_pending_sources* (**32**,
including the active reader). HTTP caps serialized heads and inline bodies
waiting for their turn with *max_pending_response_bytes* (**1 MiB**). Readers
with empty prefixes still count; source-owned memory and files do not become
inline bytes just because their contents will eventually reach the wire.

The bundled transports pause input at 75% of a budget and resume at 50%, rounded
up and down respectively. A received pipeline stops between requests too; its
remaining bytes wait in one bounded receive block. Handlers already running can
still complete. If their output exhausts a hard budget, the connection closes.
Workers never wait synchronously for output credit.

*max_connections* defaults to **65535**, shared by all workers of one server.
Excess sockets close before allocating their engines and TLS state. A closed
socket with unfinished async work keeps its reservation until the context can
retire, so reconnecting cannot bypass the live-work limit. Independent servers
have independent budgets. The maximum does not preallocate connections.

*inactivity_period* defaults to **60 seconds**. Only positive socket reads or
writes renew it, including TLS handshake bytes. Expiration aborts the socket;
queued responses and suspended handlers do not count as progress. Recording a
context's activity only sets a flag under its existing lock; this needs no clock
read or new atomic operation. Workers query the clock for maintenance scheduling
and sample activity at intervals of the smaller of the
configured period and **250 ms**. The first sample starts the idle clock, and a
sample observing activity renews it. A silent connection therefore expires no
earlier than its configured period and normally within two extra sample intervals
(at most **500 ms**), plus scheduling and scan delay. Expiration also applies during a pause or a graceful
close. It does not forcibly cancel a handler that external code may still resume.
A peer sending occasional bytes can keep this period alive: it is not a total
request deadline or minimum transfer rate.

For example, configure transport and HTTP budgets independently:

```cpp
martianlabs::doba::transport::server::tcp_policies io;
io.ip = "0.0.0.0";
io.port = "8080";
io.max_connections = 65535;
io.inactivity_period = std::chrono::seconds{60};
io.max_pending_sources = 32;
martianlabs::doba::protocol::http::v11::policies http;
http.max_pending_requests = 128;
http.max_pending_response_bytes = 1024 * 1024;
http_server<> server(io, http);
```

Zero disables each new limit and the existing pending-request limit. Negative
inactivity periods are invalid; heads, transport receive/send capacities and TLS
BIO capacities must remain positive. Transport construction validates its
policies; the public HTTP/HTTPS server constructs its transport on the first
*start()*, so invalid transport settings fail there, before listening. Servers
copy configuration: later changes to the original policy object do not update
a running server. Byte budgets count logical queued bytes and reservations,
not allocator capacity or total process memory. These limits cover framework queues, not arbitrary handler allocations,
external operations or all temporary disk use. Cache and application-owned
storage need their own budgets.

These limits sit on different sides of the protocol/transport boundary.
The [async routes chapter](03-http-1-1.md#async-routes) shows why responses
may wait for their turn; the [transport chapter](04-transports-and-tls.md)
explains the TCP side.

<a name="start-and-stop"></a>
<h2>
  <a href="../../resources/docs/guide/h2-start-and-stop.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-start-and-stop.svg" alt="Start and stop">
  </a>
  <a href="../../resources/docs/guide/h2-start-and-stop-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-start-and-stop-dark.svg" alt="Start and stop">
  </a>
</h2>

Register routes before *start*. Starting opens the listener; doba rejects
new routes while the server is running. The
[https_hello_world example](../../examples/http/v11/https_hello_world/README.md)
uses *signaler::wait* to hold the process until Ctrl+C, then stops the server:

```cpp
http_server.start();
signaler::wait();
http_server.stop();
```

With the bundled TCP and TLS transports, *stop* quiesces incoming work,
waits for pending async handlers, then stops transport workers. The server
destructor also calls *stop*, but an explicit call makes the shutdown point
clear. Call it from the control thread; the bundled transports reject a
quiesce request from one of their workers. If *start* fails, it propagates
the error.

That's the working set: choose separate transport and HTTP limits, keep
storage thresholds distinct from acceptance limits, and stop the server
deliberately. The [guide index](README.md) is here whenever you need to
jump back to another chapter.
