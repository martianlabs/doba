<a name="custom-http-response"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/custom_http_response/h1-custom-http-response.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_response/h1-custom-http-response.svg" alt="Custom HTTP response">
  </a>
  <a href="../../../../resources/examples/http/v11/custom_http_response/h1-custom-http-response-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_response/h1-custom-http-response-dark.svg" alt="Custom HTTP response">
  </a>
</h1>

This server supplies its own response type to *http_server*. The
*json_200* method builds a JSON response through doba's HTTP/1.1 response.
The class also returns its own type from every fluent method required by
the [response contract](../../../../include/protocol/http/v11/contracts.h).
The default request and router types remain in use.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/custom_http_response/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_response/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/custom_http_response/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_response/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

Build the *custom_http_response* target and run it as described in the
[HTTP/1.1 examples](../README.md). Then send:

```text
curl -i http://localhost:8080/hello
```

The response is *200 OK*, has *Content-Type: application/json*, and
contains `{"message":"hello"}`. Run one TCP example at a time.
