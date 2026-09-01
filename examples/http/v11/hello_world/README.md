# Hello world

This is the smallest HTTP/1.1 server example. It shows how to register a
route, build a response through chained mutators, and start the server.

## Route

- `GET /pipeline` returns `200 OK`, two response headers, and the body `ok`.

## Try it

```text
curl -i http://localhost:8080/pipeline
```
