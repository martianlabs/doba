# HEAD requests

This example registers the same representation handler for `GET` and `HEAD`.
The server preserves representation headers while suppressing the `HEAD` body.

## Try it

```text
curl -i http://localhost:8080/resource
curl -I http://localhost:8080/resource
```

The `HEAD` response has the same `Content-Length` as `GET` and no message body.

