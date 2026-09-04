# Asynchronous routes

This example demonstrates one coroutine route. The `background_executor` is
application scaffolding: it resumes the route on its own worker thread after
the handler suspends. Doba supplies the request lifetime and cancellation token
to the coroutine.

## Route

- `GET /work` suspends once and returns `completed /work`.

## Try it

```text
curl -i http://localhost:8080/work
```

The final response is `200 OK` with a `completed /work` body.

