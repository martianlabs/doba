# Parametrized routes

This example registers a route with typed path parameters. Doba converts the
`id` segment to `std::uint64_t` and the `detailed` segment to `bool` before
invoking the handler. A path only matches when every conversion succeeds.

## Route

- `GET /resources/:id/:detailed` returns the selected resource.
- `detailed` accepts `true`, `false`, `1`, or `0` case-insensitively.

## Try it

```text
curl -i http://localhost:8080/resources/42/true
curl -i http://localhost:8080/resources/42/false
```

An invalid typed segment does not match the route:

```text
curl -i http://localhost:8080/resources/not-a-number/true
```
