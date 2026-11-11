<a name="hello-world"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/hello_world/h1-hello-world.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/hello_world/h1-hello-world.svg" alt="Hello world">
  </a>
  <a href="../../../../resources/examples/http/v11/hello_world/h1-hello-world-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/hello_world/h1-hello-world-dark.svg" alt="Hello world">
  </a>
</h1>

This is the smallest HTTP/1.1 server example. It shows how to register a
route, build a response through chained mutators, and start the server.

<a name="route"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/hello_world/h2-route.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/hello_world/h2-route.svg" alt="Route">
  </a>
  <a href="../../../../resources/examples/http/v11/hello_world/h2-route-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/hello_world/h2-route-dark.svg" alt="Route">
  </a>
</h2>

- `GET /hello` returns `200 OK`, two response headers, and the body `ok`.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/hello_world/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/hello_world/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/hello_world/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/hello_world/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i http://localhost:8080/hello
```
