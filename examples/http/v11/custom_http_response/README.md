# Custom HTTP response

This server supplies its own response type to *http_server*. The
*json_200* method builds a JSON response through doba's HTTP/1.1 response.
The class also returns its own type from every fluent method required by
the [response contract](../../../../include/protocol/http/v11/contracts.h).
The default request and router types remain in use.

## Try it

Build the *custom_http_response* target and run it as described in the
[HTTP/1.1 examples](../README.md). Then send:

```text
curl -i http://localhost:8080/hello
```

The response is *200 OK*, has *Content-Type: application/json*, and
contains `{"message":"hello"}`. Run one TCP example at a time.
