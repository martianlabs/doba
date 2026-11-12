<a name="file-download"></a>
<h1>
  <a href="../../../../resources/examples/http/v11/file_download/h1-file-download.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/file_download/h1-file-download.svg" alt="File download">
  </a>
  <a href="../../../../resources/examples/http/v11/file_download/h1-file-download-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/file_download/h1-file-download-dark.svg" alt="File download">
  </a>
</h1>

This example opens one configured file for each `GET /download` request.
It moves the file handle into `common::reader` and passes its exact size to
`response::set_body(reader&&, length)`. Doba sends the file without loading
it into a response writer.

<a name="try-it"></a>
<h2>
  <a href="../../../../resources/examples/http/v11/file_download/h2-try-it.svg#gh-light-mode-only">
    <img src="../../../../resources/examples/http/v11/file_download/h2-try-it.svg" alt="Try it">
  </a>
  <a href="../../../../resources/examples/http/v11/file_download/h2-try-it-dark.svg#gh-dark-mode-only">
    <img src="../../../../resources/examples/http/v11/file_download/h2-try-it-dark.svg" alt="Try it">
  </a>
</h2>

Run `file_download` with a root directory and a relative file path:

```sh
file_download examples/http/v11/file_download README.md
curl -i http://localhost:8080/download
```

The response is `200 OK`, has a `Content-Length` matching the file size,
and contains this README. If the file is missing, the route returns `404`.

Linux supports independent SO_REUSEPORT workers; 0 selects one worker.
This example uses synchronous handlers and TCP prefix/source. The same engine
supports cooperative handlers; see [async_routes](../async_routes/README.md).
For TLS use `https_server<>` and account for its finite source budget.

The HTTP engine accepts pipelined requests behind a download and retains their
responses in order. It emits later responses after the reader drains; it never
submits later nonempty output against a pending transport reader. TCP reads
bounded blocks and owns the reader through its final socket batch. TLS consumes
the reader synchronously within its finite plaintext/ciphertext batch limits.
