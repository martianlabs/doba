# Custom HTTP request

This server supplies its own request type to *http_server*. Its *from*
factory builds doba's HTTP/1.1 request, then records whether the
*X-Client-Tag* header was present. The handler reads that extra value.
The default response and router types remain in use.

## Try it

Build the *custom_http_request* target and run it as described in the
[HTTP/1.1 examples](../README.md). Then send:

```text
curl http://localhost:8080/hello
curl -H "X-Client-Tag: demo" http://localhost:8080/hello
```

The first response body is *untagged*; the second is *tagged*. Run one
TCP example at a time because they use the same port.
