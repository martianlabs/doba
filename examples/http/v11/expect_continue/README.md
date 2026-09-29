# Expect continue

This example accepts a body only after the decoder has processed the request
headers. `Expect: 100-continue` receives an interim response before `curl`
sends the body; an unsupported expectation receives `417 Expectation Failed`.

## Try it

```text
curl -v -H "Expect: 100-continue" --data-binary "doba" \
  http://localhost:8080/echo
curl -i -H "Expect: unsupported" -X POST http://localhost:8080/echo
```

The verbose output of the first command includes `HTTP/1.1 100 Continue`; the
second command returns `417 Expectation Failed`.

