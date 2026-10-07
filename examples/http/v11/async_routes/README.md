# Asynchronous routes

This example registers a coroutine route and a synchronous route on the same
HTTP/1.1 server. `GET /slow` suspends for about 50 ms and resumes on the
transport worker. `GET /fast` completes synchronously. Responses on one
connection remain in request order even if `/fast` finishes first.

```text
curl -i http://localhost:8080/slow
curl -i http://localhost:8080/fast
```

Both routes return `200 OK` with their route name as the body. Doba schedules
`sleep_for` on the transport worker that received the request. `server.stop()`
waits for pending coroutines to finish before stopping the workers.
