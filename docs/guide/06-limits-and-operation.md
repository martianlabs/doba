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

The first **server<>** argument configures the transport. The second
configures the HTTP engine. The
[request_limits example](../../examples/http/v11/request_limits/README.md)
sets a five-byte body limit in its
[source](../../examples/http/v11/request_limits/main.cpp):

```cpp
policies limits;
limits.max_content_length = 5;
server<> http_server({.ip = "0.0.0.0", .port = "8080"}, limits);
```

The address belongs to TCP; *max_content_length* belongs to HTTP. You can
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

*max_content_length* defaults to **16 MiB**. It caps accepted request body
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

An accepted body does not have to stay in RAM. The HTTP policy
*request_body_memory_threshold* controls when doba spills an incoming body
to a temporary file; it defaults to **16 KiB**. This is a storage threshold,
not a request-size limit. *max_content_length* still decides whether the
body is accepted.

Responses have a similar distinction. *response_body_inline_capacity* is
the HTTP response's inline body capacity, while a body writer has its own
storage options. The [large_response_bodies example][large-bodies] uses a
raw writer that spills after **1024** bytes, then builds an
**8192-byte** response. Its
[source](../../examples/http/v11/large_response_bodies/main.cpp) starts
the writer like this:

[large-bodies]: ../../examples/http/v11/large_response_bodies/README.md

```cpp
byte_storage_options options{.spill_threshold = 1024, .spill_dir = {}};
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
- **Queued output.** The TCP policy *max_send_buffer_size* defaults to
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
