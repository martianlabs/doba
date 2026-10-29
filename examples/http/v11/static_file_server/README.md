# Static file server

The [source](main.cpp) mounts one directory twice: **/assets/** uses an opt-in
small-file cache and **/downloads/** keeps direct file reads. The cache is doba's
protocol-independent [common::cache](../../../common/cache/README.md), specialized
for immutable file representations. See the
[controller reference](../../../../docs/reference/protocol/http/v11/static_file_server.md)
for every option.

Build the **static_file_server** target, then run from the repository root:

```console
static_file_server examples/http/v11/static_file_server/public
```

An optional port follows the root. Add **--no-cache** before the port to disable
the asset cache for comparison:

```console
static_file_server examples/http/v11/static_file_server/public --no-cache 8081
```

```console
curl -i http://localhost:8080/assets/hello.txt
curl -I http://localhost:8080/assets/hello.txt
curl -i -H "Range: bytes=0-4" http://localhost:8080/assets/hello.txt
curl -i -H 'If-None-Match: "paste-the-returned-etag"' http://localhost:8080/assets/hello.txt
curl --path-as-is -i http://localhost:8080/assets/../hello.txt
```

GET and HEAD share file length and validators. Cached snapshots have strong
SHA-256 ETags; direct streams have weak metadata ETags. Both emit Last-Modified
and, by default, Cache-Control: no-cache. That HTTP policy allows revalidation;
it is independent of the internal cache's one-second retention TTL. Every cache
hit compares file identity, device, size, modification/change timestamps, flags
and link count. Equal attributes reuse cached bytes without rereading the file.
No subscriptions or background callbacks are used.

The example enables precompressed asset negotiation. Publish **hello.txt.br** or
**hello.txt.gz** alongside the original and request the corresponding encoding.
Variants have their own validators and include Vary: Accept-Encoding. doba does
not create compressed files or verify their decoded contents.

The default cache admits up to 1024 entries, 512 MiB of declared live cost and 16
concurrent loads. Files above 1 MiB, or rejected by the budget, use direct
streaming. A request coinciding with an active load also streams directly, without
waiting for that loader. Expiration reloads on the next request. To invalidate immediately,
retain the supplied shared cache and call clear(); controller invalidate(relative)
also removes a filename and its compressed siblings. Invalidating does not revoke
bytes already retained by a response. Evicted snapshots and reservations remain
charged until their consumers/loaders release them. Declared cost excludes keys,
allocator/control-block overhead and excess string capacity; it is not a process
RSS limit.

Publish complete files by atomic replacement. Attributes are checked before and
after a load and when adopting a concurrently cached value. In-flight responses
retain their old snapshots. Missing compressed variants use confined attribute probes and are never cached;
new variants are discovered without waiting for a directory timestamp or TTL.
Remote metadata caching or a write preserving all observed attributes can hide a
change; filesystem semantics still apply. Direct streams own a handle, not a
snapshot: concurrent writes can alter bytes, growth is ignored, and premature EOF
closes the connection.
Reads and cold loads remain synchronous. The existing transport buffers and send
algorithm are unchanged.

Only regular files are served. Missing files/directories return 404. Traversal,
links, Windows reparse points, hidden components and configured exclusions return
403; other I/O failures before sending return 500. Paths are decoded once.
Literal/parameterized routes retain precedence; other methods use 405 and Allow.

Simple byte ranges return 206 or 416. Multiple ranges are ignored. If-Range accepts
strong tags; dates and weak tags get the full response. Index files, directory
redirects, SPA fallback, MIME overrides and exclusions are separate policy options;
navigation options are disabled by default and no directory listing is provided.

For repeatable measurements, use the
[HttpArena adapter](../../../../benchmarks/http/v11/httparena/README.md) and its
static-tls profile.
