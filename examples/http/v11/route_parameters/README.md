# Route parameters

This example uses named path segments and lets the router convert them to the
types declared by the route handler: `int`, `bool`, `double`, and
`std::string`.

## Route

- `GET /items/:id/:enabled/:score/:name` returns every converted value.

The boolean parameter accepts `true`, `false`, `1`, and `0`. A parameter that
cannot be converted prevents this parametrized route from matching.

## Try it

```text
curl -i http://localhost:8080/items/42/true/3.5/widget
```
