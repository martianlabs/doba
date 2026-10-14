# Custom HTTP router

This server supplies its own router type to *http_server*. The router
stores handlers in doba's HTTP router and treats */alias* as */hello*
when selecting a route or listing allowed methods. Its *match* method
returns the shared *router_match_result* type. The default request and
response types remain in use.

## Try it

Build the *custom_http_router* target and run it as described in the
[HTTP/1.1 examples](../README.md). Then send:

```text
curl -i http://localhost:8080/hello
curl -i http://localhost:8080/alias
curl -i -X POST http://localhost:8080/alias
```

Both GET requests return *200 OK* with *hello*. POST returns
*405 Method Not Allowed* with *Allow: GET*. Run one TCP example at a time.
