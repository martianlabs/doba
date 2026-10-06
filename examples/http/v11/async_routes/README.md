# Asynchronous routes

This example registers a coroutine route and a synchronous route on the same
HTTP/1.1 server. `GET /slow` suspends for about 50 ms and resumes on the
example's timer thread. `GET /fast` completes synchronously. Responses on one
connection remain in request order even if `/fast` finishes first.

```text
curl -i http://localhost:8080/slow
curl -i http://localhost:8080/fast
```

Both routes return `200 OK` with their route name as the body. The timer is
owned by the example and stays alive until `server.stop()` has waited for its
coroutines. Doba does not provide a scheduler or cancel a suspended awaitable;
the awaitable must eventually resume for `stop()` to finish.
