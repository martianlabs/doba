# Response body writers

This example builds response bodies incrementally with raw and chunked
`body_writer` instances. Writers accept text and `std::byte` spans.

## Routes

- `GET /raw` writes two text parts and produces a Content-Length response.
- `GET /binary` writes a binary span through a raw writer.
- `GET /chunked` emits two chunks and the terminating chunk.

Moving a writer into the response selects the matching framing headers.
Chunked writers must be finalized with `end()` after the last write.

## Try it

```text
curl -i http://localhost:8080/raw
curl --output response.bin http://localhost:8080/binary
curl --raw -i http://localhost:8080/chunked
```
