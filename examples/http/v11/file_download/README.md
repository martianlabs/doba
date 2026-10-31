# File download

This example opens one configured file for each `GET /download` request.
It moves the file handle into `common::reader` and passes its exact size to
`response::set_body(reader&&, length)`. Doba sends the file without loading
it into a response writer.

## Try it

Run `file_download` with a root directory and a relative file path:

```sh
file_download examples/http/v11/file_download README.md
curl -i http://localhost:8080/download
```

The response is `200 OK`, has a `Content-Length` matching the file size,
and contains this README. If the file is missing, the route returns `404`.

Linux supports independent SO_REUSEPORT workers; 0 selects one worker.
The new transport supports synchronous callbacks and prefix/source;
TLS and asynchronous routes are outside this phase.

On Windows and Linux the output reader is terminal for its input call. Do not
pipeline a later response behind a download reader in the same RX block.
Send the next request after the download drains. The transport reads bounded
blocks and owns the reader until its final socket batch completes.
