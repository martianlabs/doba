# Static file server

This example mounts one directory at `/assets/` and `/downloads/`, each through
an owned `static_file_server` controller.

## Run

Build the `static_file_server` target. From the repository root, run:

```sh
static_file_server examples/http/v11/static_file_server/public
```

Then:

```sh
curl -i http://localhost:8080/assets/hello.txt
curl -I http://localhost:8080/assets/hello.txt
curl http://localhost:8080/downloads/index.html
curl -i http://localhost:8080/assets/
curl --path-as-is -i http://localhost:8080/assets/../hello.txt
```

The text request returns `200`, `Content-Type: text/plain`, and
`Hello from doba.`. HEAD has the same file length and no body. The second mount
serves the sample HTML. The directory request returns `404`; traversal returns
`403`. Files are opened afresh for every request, so edits appear on subsequent
requests without restarting the server.

The controller registers GET and HEAD wildcard routes. Existing literal and
parametrized routes keep their precedence over a wildcard. Other methods use
the router's existing `405` and `Allow` handling.

Responses read directly from an owned file handle through the transport's
bounded buffers. No file contents are cached or spooled. A response uses the
size captured when opening the file: growth is ignored and premature EOF aborts
the connection. Concurrent in-place writes can change bytes during a download;
the handle provides lifetime ownership, not a content snapshot.

Only regular files are served. Missing files and directories return `404`;
traversal, links, Windows reparse points and access denial return `403`; other
I/O failures before sending return `500`. The request path is already decoded
by doba and is never decoded again. Backslashes, drive paths, alternate streams,
and Windows device names are rejected.

There is no automatic index, directory listing, redirect, SPA fallback,
compression, range serving, or application cache. Range is ignored and a full
`200` response is sent. ETag and Last-Modified are not generated. `If-Match: *`
checks existence, an explicit unmatched If-Match returns `412`, and
`If-None-Match: *` returns `304` for an existing file. Date conditions have no
modification date to evaluate in this version.

File reads are synchronous and may block a transport worker on slow storage.
Caching and further file delivery optimizations are deferred.
