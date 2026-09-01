# Request information

This example reads the request method, absolute path, request-target form,
host, port, host type, and connection-close preference from `request`.

## Route

- `GET /request` returns the parsed request information as plain text.

The returned string views remain backed by the request object and are consumed
inside the route handler.

## Try it

```text
curl -i -H "Connection: close" http://localhost:8080/request
```
