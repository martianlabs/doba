<a name="request-information"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/request_information/h1-request-information.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_information/h1-request-information.svg" alt="Request information">
  </a>
  <a href="../../../../resources/examples/http/v11/request_information/h1-request-information-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_information/h1-request-information-dark.svg" alt="Request information">
  </a>
</h1>

This example reads the request method, absolute path, request-target form,
host, port, host type, and connection-close preference from `request`.

<a name="route"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/request_information/h2-route.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_information/h2-route.svg" alt="Route">
  </a>
  <a href="../../../../resources/examples/http/v11/request_information/h2-route-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_information/h2-route-dark.svg" alt="Route">
  </a>
</h2>

- `GET /request` returns the parsed request information as plain text.

The returned string views remain backed by the request object and are consumed
inside the route handler.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/request_information/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/request_information/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/request_information/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/request_information/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

```text
curl -i -H "Connection: close" http://localhost:8080/request
curl -i --request-target "http://localhost:8080/request" \
  http://localhost:8080/request
```

The first request uses origin-form. The second uses absolute-form and is
routed to the same absolute path.
