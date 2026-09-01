# Standard HTTP behaviors

This example registers the same representation handler for `GET` and `HEAD`.
It also exposes server behavior that does not require an explicit route.

## Routes and automatic responses

- `GET /resource` returns the resource representation.
- `HEAD /resource` returns the corresponding headers without a message body.
- An unknown path produces `404 Not Found`.
- A known path with an unsupported method produces `405 Method Not Allowed`.
- `OPTIONS *` is handled directly by the server.

## Try it

```text
curl -i http://localhost:8080/resource
curl -I http://localhost:8080/resource
curl -i -X POST http://localhost:8080/resource
curl -i --request-target "*" -X OPTIONS http://localhost:8080
```
