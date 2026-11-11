<a name="custom-http-router"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/custom_http_router/h1-custom-http-router.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_router/h1-custom-http-router.svg" alt="Custom HTTP router">
  </a>
  <a href="../../../../resources/examples/http/v11/custom_http_router/h1-custom-http-router-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_router/h1-custom-http-router-dark.svg" alt="Custom HTTP router">
  </a>
</h1>

This server supplies its own router type to *http_server*. The router
stores handlers in doba's HTTP router and treats */alias* as */hello*
when selecting a route or listing allowed methods. Its *match* method
returns the shared *router_match_result* type. The default request and
response types remain in use.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/custom_http_router/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_router/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/custom_http_router/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/custom_http_router/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

Build the *custom_http_router* target and run it as described in the
[HTTP/1.1 examples](../README.md). Then send:

```text
curl -i http://localhost:8080/hello
curl -i http://localhost:8080/alias
curl -i -X POST http://localhost:8080/alias
```

Both GET requests return *200 OK* with *hello*. POST returns
*405 Method Not Allowed* with *Allow: GET*. Run one TCP example at a time.
