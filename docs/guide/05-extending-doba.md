<a name="extending-doba"></a>
<h1>
  <a href="../../resources/docs/guide/h1-extending-doba.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h1-extending-doba.svg" alt="Extending doba">
  </a>
  <a href="../../resources/docs/guide/h1-extending-doba-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h1-extending-doba-dark.svg" alt="Extending doba">
  </a>
</h1>

Want a protocol that isn't HTTP? Or a transport that doesn't use sockets?
Pick the side you want to replace and keep the other one. The
[core model](02-core-model.md) covers the contracts; here we'll follow two
small examples that put them to work.

<a name="build-the-examples"></a>
<h2>
  <a href="../../resources/docs/guide/h2-build-the-examples.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-build-the-examples.svg" alt="Build the examples">
  </a>
  <a href="../../resources/docs/guide/h2-build-the-examples-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-build-the-examples-dark.svg" alt="Build the examples">
  </a>
</h2>

From the repository root, build both demos. They use plain TCP or memory,
so OpenSSL is not needed:

```sh
cmake -S . -B out/build/guide
cmake --build out/build/guide --config Release --target custom_protocol
cmake --build out/build/guide --config Release --target custom_transport
```

As in the earlier chapters, *--config Release* selects the configuration
for generators such as Visual Studio.

<a name="your-own-protocol"></a>
<h2>
  <a href="../../resources/docs/guide/h2-your-own-protocol.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-your-own-protocol.svg" alt="Your own protocol">
  </a>
  <a href="../../resources/docs/guide/h2-your-own-protocol-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-your-own-protocol-dark.svg" alt="Your own protocol">
  </a>
</h2>

The [custom_protocol example](../../examples/protocol/custom_protocol/README.md)
speaks a tiny line-based language. Send a line, get that line back with
**echo:** in front. Its
[source](../../examples/protocol/custom_protocol/main.cpp) defines one
engine per connection. The engine stores the output callback and ignores
the close callback because this demo never requests a close:

```cpp
struct policies_type {};

void set_on_send(protocol::send_delegate output) {
  on_send_ = std::move(output);
}
void set_on_close(std::function<void()>) {}
```

The input method appends each new block to engine-owned storage and sends
complete lines:

```cpp
void on_bytes_received(std::string_view input) {
  input_.append(input);
  std::size_t consumed = 0;
  for (;;) {
    const std::string_view remaining(input_.data() + consumed,
                                      input_.size() - consumed);
    const auto end = remaining.find('\n');
    if (end == std::string_view::npos) break;
    const auto line = remaining.substr(0, end + 1);
    on_send_("echo: ", line, nullptr);
    consumed += line.size();
  }
  input_.erase(0, consumed);
}
```

Here input_ is a std::string member. Incomplete lines remain in that
string. The transport delivers only fresh bytes, and may overwrite its
receive buffer as soon as the callback returns. A production line protocol
must enforce its own maximum line length.

<a name="run-it-over-tcp"></a>
<h2>
  <a href="../../resources/docs/guide/h2-run-it-over-tcp.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-run-it-over-tcp.svg" alt="Run it over TCP">
  </a>
  <a href="../../resources/docs/guide/h2-run-it-over-tcp-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-run-it-over-tcp-dark.svg" alt="Run it over TCP">
  </a>
</h2>

TCP needs a fresh engine for each connection, so the example gives the
[core server](../reference/server.md) a factory and a concrete TCP transport.
No HTTP server is involved:

```cpp
auto create_engine = []() { return custom_protocol{}; };
using transport_type = transport::tcp<custom_protocol,
                                      decltype(create_engine)>;
martianlabs::doba::server<custom_protocol, transport_type> server(
    {.ip = "0.0.0.0", .port = "8080"}, create_engine);
```

The example starts the server, waits for a stop signal, and then stops it.
On Linux with a single-configuration generator, run:

```sh
./out/build/guide/examples/protocol/custom_protocol/custom_protocol
```

With Visual Studio on Windows, run:

```powershell
$build = '.\out\build\guide\examples\protocol\custom_protocol'
& "$build\Release\custom_protocol.exe"
```

Send **hello** followed by a newline from a TCP client to **localhost:8080**.
You should receive **echo: hello** followed by a newline. Stop the server
with Ctrl+C when you're done.

<a name="your-own-transport"></a>
<h2>
  <a href="../../resources/docs/guide/h2-your-own-transport.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-your-own-transport.svg" alt="Your own transport">
  </a>
  <a href="../../resources/docs/guide/h2-your-own-transport-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-your-own-transport-dark.svg" alt="Your own transport">
  </a>
</h2>

The [custom_transport example][custom-transport] flips the experiment: it
keeps doba's HTTP/1.1 engine and supplies a tiny in-memory transport. Its
[source](../../examples/transport/server/custom_transport/main.cpp) takes
the engine factory in its constructor. In *start*, it creates the engine and
wires up its output and close callbacks:

[custom-transport]: ../../examples/transport/server/custom_transport/README.md

```cpp
connection_.reset(new ENty(create_engine_()));
connection_->set_on_send(
    [this](std::string_view head, std::string_view body,
           std::unique_ptr<common::reader> source) {
      output_->append(head);
      output_->append(body);
      if (source) source->read_all(*output_);
    });
connection_->set_on_close([this]() { close_requested_ = true; });
on_connection_();
```

The output callback appends the response head and body to the supplied
string, then drains any body reader. *start* feeds one fixed
**GET /hello** request; *stop* destroys the engine and announces the
disconnection. The caller owns the output string and keeps it alive until
the transport stops.

<a name="run-http-in-memory"></a>
<h2>
  <a href="../../resources/docs/guide/h2-run-http-in-memory.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-run-http-in-memory.svg" alt="Run HTTP in memory">
  </a>
  <a href="../../resources/docs/guide/h2-run-http-in-memory-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-run-http-in-memory-dark.svg" alt="Run HTTP in memory">
  </a>
</h2>

The HTTP server accepts the transport as its final template argument. The
example creates the output string first, then passes its address as the
transport policy:

```cpp
std::string output;
martianlabs::doba::http_server<http::request, http::response, routes_type,
                               engine_type, custom_transport>
    server(&output);
```

Run the already-built example. On Linux:

```sh
./out/build/guide/examples/transport/server/custom_transport/custom_transport
```

With Visual Studio on Windows:

```powershell
$build = '.\out\build\guide\examples\transport\server\custom_transport'
& "$build\Release\custom_transport.exe"
```

It prints an HTTP response that starts with **HTTP/1.1 200** and ends with
**hello from memory**. There is no listener or network client to start.

<a name="keep-the-boundary-safe"></a>
<h2>
  <a href="../../resources/docs/guide/h2-keep-the-boundary-safe.svg#gh-light-mode-only">
    <img src="../../resources/docs/guide/h2-keep-the-boundary-safe.svg" alt="Keep the boundary safe">
  </a>
  <a href="../../resources/docs/guide/h2-keep-the-boundary-safe-dark.svg#gh-dark-mode-only">
    <img src="../../resources/docs/guide/h2-keep-the-boundary-safe-dark.svg" alt="Keep the boundary safe">
  </a>
</h2>

The examples are small on purpose. When you move beyond them, keep three
runtime rules from the [contracts](02-core-model.md#protocol-contract) in
view:

- **Input.** Deliver only newly received bytes, with their pointer and size.
  The engine processes or copies them before returning. Incomplete protocol
  messages belong to the engine; the transport may immediately reuse its buffer.
- **Output.** The two views passed to *send_delegate* do not own their bytes.
  If a transport sends later, it must preserve that data; it takes ownership
  of an optional body reader.
- **Threads and closure.** Input calls for one connection are serialized.
  Output can arrive from different threads, and a close request must stop
  further input for that connection.

HTTP also exposes an optional flow-control extension. The bundled TCP and TLS
transports recognize these engine methods:

```cpp
void set_input_flow_control(std::function<bool()> pressure,
                            std::function<void()> ready);
bool input_paused() const noexcept;
void resume_input();
```

Install the hooks before delivering input. The pressure query reports transport
output pressure. Once *input_paused()* is true, suspend further input delivery;
the HTTP engine owns a copy of the unprocessed remainder of one receive block.
It pauses between messages, allowing a message already in progress to finish.

The *ready* callback may run on any completing thread. Schedule and coalesce a
wakeup onto the connection's serialized worker, retaining wakeups that arrive
during a receive call. Invoke *resume_input()* outside output locks and serialize
it with receive and stop calls. A wakeup can be spurious: recheck every budget,
and service deferred input even when no new network bytes arrive. Honor close
requests and TLS control records processed during resumption too.

Keep callbacks lifetime-safe through accepted asynchronous work; weak context
ownership avoids cycles in the bundled transports. A custom transport owns its
admission, inactivity and output budgets. Without these optional hooks, HTTP
still enforces hard limits, but cannot cooperatively pause that transport.

Custom HTTP requests accept a final `borrow_bytes` argument in `from(...)`.
The decoder supplies true so fields can reference its immutable input;
standalone factory calls default to owned field storage. Requests expose
`borrows_bytes()` and `make_owned()` for copying before async invocation.
`rebase_bytes(previous_address, current_buffer) noexcept` supports residual
relocation before dispatch; `body_is_stored()` and `body_wire_size()` determine
whether fragmented body bytes need residual storage. Borrowed input must
remain immutable; the standard request copies percent-encoded paths before
decoding them.
Their `begin_body(memory_threshold, declared_size)`,
`append_body(wire_bytes)` and
`finish_body(chunked, content_length, extension_limit, trailer_limit)`
operations support construction across receives; a false result reports a
storage failure. The decoder calls the factory once the head is complete,
but dispatches the request only after its body is complete. Deriving from the
built-in request supplies these operations.

Custom HTTP responses provide an owning factory
`from(max_head_size, body_memory_threshold)` and a synchronous factory
`from(max_head_size, body_memory_threshold, head_storage, body_storage)`.
The last two arguments are references to separate engine-owned `std::string`
objects. The synchronous response starts with fresh message state and writes
directly into them, reusing their capacity. These borrowed strings must outlive
serialization and the transport's output capture. The limits remain
operational bounds; no default capacity is reserved. Deriving from the
standard response supplies both storage modes. The custom response example
forwards both factories.

The in-memory demo handles one fixed request synchronously. It shows where
the pieces meet; a real network transport also needs connection management,
buffering, and safe output delivery. Head back to the [guide index](README.md)
when you're ready for the final chapter.
