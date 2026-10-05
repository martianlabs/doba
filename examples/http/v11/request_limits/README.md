# Request limits

This example passes HTTP/1.1 `policies` as the second argument to `server`.
It accepts request bodies of at most five bytes. Larger bodies are rejected
before the `POST /upload` handler runs.

## Try it

```sh
curl -i --data-binary "12345" http://localhost:8080/upload
curl -i --data-binary "123456" http://localhost:8080/upload
```

The first request returns `200 OK` with `accepted`. The second returns
`413 Content Too Large`.
