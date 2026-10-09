<a name="extending-doba"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h1-extending-doba-dark.svg">
    <img src="../../resources/docs/guide/h1-extending-doba.svg" alt="Extending doba">
  </picture>
</h1>

Want a protocol that isn't HTTP? Or a transport that doesn't use sockets?
Pick the side you want to replace and keep the other one. The
[core model](02-core-model.md) covers the contracts; here we'll follow two
small examples that put them to work.

<a name="build-the-examples"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-build-the-examples-dark.svg">
    <img src="../../resources/docs/guide/h2-build-the-examples.svg" alt="Build the examples">
  </picture>
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
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-your-own-protocol-dark.svg">
    <img src="../../resources/docs/guide/h2-your-own-protocol.svg" alt="Your own protocol">
  </picture>
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

The input method looks for complete lines. It sends each one and returns the
number of bytes consumed:

```cpp
std::size_t on_bytes_received(const char* buffer, std::size_t size,
                              std::size_t) {
  std::size_t consumed = 0;
  while (consumed < size) {
    const std::string_view remaining(buffer + consumed, size - consumed);
    const std::size_t end = remaining.find('\n');
    if (end == std::string_view::npos) break;
    const std::string_view line = remaining.substr(0, end + 1);
    on_send_("echo: ", line, nullptr);
    consumed += line.size();
  }
  return consumed;
}
```

If the last line has no newline yet, the method leaves those bytes
unconsumed. TCP keeps them in the connection's receive buffer and tries
again when more data arrives. A line that fills the buffer without a newline
cannot make progress, so TCP closes that connection. This is why the return
value matters as much as the parser.

<a name="run-it-over-tcp"></a>
<h2>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-run-it-over-tcp-dark.svg">
    <img src="../../resources/docs/guide/h2-run-it-over-tcp.svg" alt="Run it over TCP">
  </picture>
</h2>

TCP needs a fresh engine for each connection, so the example gives it a
factory. No HTTP server is involved:

```cpp
auto create_engine = []() { return custom_protocol{}; };
transport::tcp<custom_protocol, decltype(create_engine)> server(
    {.ip = "0.0.0.0", .port = "8080"}, create_engine);
```

The example registers connection callbacks, starts TCP, waits for a stop
signal, and then stops it. On Linux with a single-configuration generator,
run:

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
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-your-own-transport-dark.svg">
    <img src="../../resources/docs/guide/h2-your-own-transport.svg" alt="Your own transport">
  </picture>
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
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-run-http-in-memory-dark.svg">
    <img src="../../resources/docs/guide/h2-run-http-in-memory.svg" alt="Run HTTP in memory">
  </picture>
</h2>

The HTTP server accepts the transport as its final template argument. The
example creates the output string first, then passes its address as the
transport policy:

```cpp
std::string output;
http::server<http::request, http::response, routes_type, engine_type,
             custom_transport>
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
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../resources/docs/guide/h2-keep-the-boundary-safe-dark.svg">
    <img src="../../resources/docs/guide/h2-keep-the-boundary-safe.svg" alt="Keep the boundary safe">
  </picture>
</h2>

The examples are small on purpose. When you move beyond them, keep three
runtime rules from the [contracts](02-core-model.md#protocol-contract) in
view:

- **Input.** Report exactly how many bytes the engine consumed. Any
  unconsumed bytes must remain available for the next call.
- **Output.** The two views passed to *send_delegate* do not own their bytes.
  If a transport sends later, it must preserve that data; it takes ownership
  of an optional body reader.
- **Threads and closure.** Input calls for one connection are serialized.
  Output can arrive from different threads, and a close request must stop
  further input for that connection.

The in-memory demo handles one fixed request synchronously. It shows where
the pieces meet; a real network transport also needs connection management,
buffering, and safe output delivery. Head back to the [guide index](README.md)
when you're ready for the final chapter.
