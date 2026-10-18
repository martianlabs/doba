<a name="limits-and-operation"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h1-limits-and-operation-dark.svg">
    <img src="../../resources/docs/guide/h1-limits-and-operation.svg" alt="Limits and operation">
  </picture>
</h1>

A busy server needs boundaries. doba gives you one set of knobs for the
transport and another for HTTP/1.1. Some reject work that is too large;
others decide how much stays in memory. Knowing which is which saves a lot
of head-scratching.

<a name="two-sets-of-policies"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-two-sets-of-policies-dark.svg">
    <img src="../../resources/docs/guide/h2-two-sets-of-policies.svg" alt="Two sets of policies">
  </picture>
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
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-limit-incoming-requests-dark.svg">
    <img src="../../resources/docs/guide/h2-limit-incoming-requests.svg" alt="Limit incoming requests">
  </picture>
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
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-memory-and-large-bodies-dark.svg">
    <img src="../../resources/docs/guide/h2-memory-and-large-bodies.svg" alt="Memory and large bodies">
  </picture>
</h2>

The decoder normally reads the transport buffer directly. Requests borrow
field bytes and inline bodies; responses own their serialized bytes.
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
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-bound-work-in-flight-dark.svg">
    <img src="../../resources/docs/guide/h2-bound-work-in-flight.svg" alt="Bound work in flight">
  </picture>
</h2>

Body size is only one way a connection can use resources. Two more limits
are worth knowing:

- **Async responses.** Once a connection uses async handling,
  *max_pending_requests* caps response positions awaiting ordered delivery.
  It defaults to **32**. When full, doba closes the connection rather than
  reserving another position. **0** removes this limit.
- **Queued output.** The TCP policy *max_buffered_send_bytes* defaults to
  **1 MiB** per connection. It bounds queued send reservations plus the
  active batch; a connection that exceeds it closes. Storage owned by a body
  source is outside this cap.

These limits sit on different sides of the protocol/transport boundary.
The [async routes chapter](03-http-1-1.md#async-routes) shows why responses
may wait for their turn; the [transport chapter](04-transports-and-tls.md)
explains the TCP side.

<a name="start-and-stop"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-start-and-stop-dark.svg">
    <img src="../../resources/docs/guide/h2-start-and-stop.svg" alt="Start and stop">
  </picture>
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
