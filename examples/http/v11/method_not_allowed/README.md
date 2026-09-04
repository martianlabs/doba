# Method not allowed

This example registers only `GET /resource`. A different method for the same
path produces `405 Method Not Allowed` and lists the accepted method in `Allow`.

## Try it

```text
curl -i -X POST http://localhost:8080/resource
```

