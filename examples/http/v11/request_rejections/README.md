# Request rejections

This server has no application routes. It demonstrates that decoder failures
become HTTP responses rather than being silently dropped.

## Try it

```text
curl -i --http1.0 http://localhost:8080/
curl -i -H "Expect: unsupported" -X POST http://localhost:8080/
```

The first request is rejected with `400 Bad Request`; the second returns
`417 Expectation Failed`.

