# Asynchronous routes

This example compares the default synchronous route policy with
`execution_policy::kAsynchronous`.

## Routes

- `GET /synchronous` runs its handler synchronously.
- `GET /asynchronous` queues its handler and returns after simulated work.

The asynchronous handler keeps the shared request and response objects alive
until it completes and the response is sent.

## Try it

```text
curl -i http://localhost:8080/synchronous
curl -i http://localhost:8080/asynchronous
```
