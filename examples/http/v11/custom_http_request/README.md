<a name="custom-http-request"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/custom_http_request/h1-custom-http-request.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_request/h1-custom-http-request.svg" alt="Custom HTTP request">
  </a>
  <a href="../../../../resources/examples/http/v11/custom_http_request/h1-custom-http-request-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_request/h1-custom-http-request-dark.svg" alt="Custom HTTP request">
  </a>
</h1>

This server supplies its own request type to *http_server*. Its *from*
factory builds doba's HTTP/1.1 request, then records whether the
*X-Client-Tag* header was present. The handler reads that extra value.
The default response and router types remain in use.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/custom_http_request/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_request/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/custom_http_request/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_request/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

Build the *custom_http_request* target and run it as described in the
[HTTP/1.1 examples](../README.md). Then send:

```text
curl http://localhost:8080/hello
curl -H "X-Client-Tag: demo" http://localhost:8080/hello
```

The first response body is *untagged*; the second is *tagged*. Run one
TCP example at a time because they use the same port.
