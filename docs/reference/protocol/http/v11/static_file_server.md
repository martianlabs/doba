<a id="static-file-server"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/static_file_server/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/static_file_server/title.svg" alt="static_file_server">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/static_file_server/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/static_file_server/title-dark.svg" alt="static_file_server">
</a>

**Header:** [include/protocol/http/v11/static_file_server.h](../../../../../include/protocol/http/v11/static_file_server.h)

Mounts regular files through GET/HEAD. See the
[static-file example](../../../../../examples/http/v11/static_file_server/README.md).
The two-argument constructor keeps direct reads. Pass static_file_policies as the
third argument for caching and optional features. The existing reader/transport
sending procedure and its buffer sizes are unchanged.

Direct readers retain handles, not snapshots: in-place writes can alter a download,
growth is ignored and premature EOF aborts the connection. Cached readers retain
immutable bytes with independent cursors. Publish complete files by atomic
replacement. Before/after metadata checks detect many writes during loading but
cannot establish a snapshot against an arbitrary concurrent writer.

A cache hit avoids reopening until TTL expiry or invalidation. File deletion,
permission revocation and changes to links can therefore take that long to become
visible. Accept that window before enabling caching for a mutable tree.
Configuration is immutable after construction; register routes before serving.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> static_file_server

[static_file_representation](#representation) · [static_file_cache](#cache) · [static_file_policies](#policies) · [static_file_server](#type-static-file-server) · [register_routes](#method-register-routes) · [invalidate](#invalidate) · [clear_cache](#clear-cache)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="representation"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> static_file_representation

```cpp
struct static_file_representation {
  std::string bytes;
  common::filesystem_metadata metadata;
  std::string etag;
};
```

Immutable published value. bytes contains the selected encoded payload; metadata comes from the open handle; etag is its quoted SHA-256. Strings start empty; metadata fields start at zero. Do not mutate a value after publication.

<a id="cache"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> static_file_cache

```cpp
using static_file_cache = common::cache<std::string, static_file_representation>;
```

The [generic common cache](../../../common/cache.md) specialized for file representations. Keys contain canonical UTF-8 root, NUL separator and decoded filename including compression suffix. Shared caches coordinate budgets across mounts. Declared cost is payload length + sizeof(static_file_representation) + 66 ETag characters; keys, allocator/control-block overhead and excess string capacity are excluded.

<a id="policies"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> static_file_policies

```cpp
struct static_file_policies { ... };
```

| Field | Default | Behavior |
| --- | --- | --- |
| cache | null shared_ptr<static_file_cache> | Direct reads; supply a shared cache to enable snapshots. |
| max_cached_file_size | std::size_t / 1048576 bytes (1 MiB) | Per-file admission threshold; larger files stream. |
| cache_control | std::string / no-cache | HTTP policy independent of internal TTL; empty omits it. |
| deny_hidden | bool / true | Deny any component starting with a dot. |
| ranges | bool / true | Simple GET byte ranges; HEAD describes the full representation. |
| precompressed | bool / false | Negotiate .br/.gz siblings and send Vary: Accept-Encoding. |
| redirect_directories | bool / false | Slash redirects, including the exact mount root. |
| index_file | std::string / empty | Optional safe filename for directories (one component). |
| spa_fallback | std::string / empty | Safe root-relative fallback for missing paths. |
| excluded | vector<string> / empty | Exact decoded relative paths and descendants to deny. |
| mime_types | vector<pair<string,string>> / empty | Case-insensitive extension/MIME overrides before built-ins. |

Policies are copied. Invalid prefixes, indexes, fallbacks, cache policies or MIME
mappings throw std::invalid_argument; inaccessible/non-directory roots throw
std::filesystem::filesystem_error. Admission rejection streams directly. A cold
HEAD may populate the cache to obtain the same strong validator as GET.

Precompressed siblings must be published consistently by the application. doba
neither compresses them nor validates decoded content. Absent Accept-Encoding
selects identity; q=0 and wildcard exclusions are respected. Ties prefer br, gzip,
identity. When identity exists but no acceptable variant is available, return 406.
Directory listing, multiple-range bodies and dynamic compression are unsupported.
SPA fallback applies to missing paths without filtering Accept or extension; use
it on a dedicated application mount. It never bypasses denied paths or I/O errors.

<a id="type-static-file-server"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> static_file_server

```cpp
class static_file_server;
```

GET serves 200/206; missing files/directories return 404; traversal, links,
reparse points, exclusions and access denial return 403; other pre-send I/O errors
return 500. Only regular files are served. Request paths are decoded once by the
HTTP decoder. MIME falls back to application/octet-stream.

Cached representations use strong SHA-256 ETags; direct streams use weak metadata
ETags. Last-Modified comes from the open handle, clamped to now. Conditions follow
HTTP precedence: If-Match, otherwise If-Unmodified-Since, then If-None-Match,
otherwise If-Modified-Since. Entity-tag lists, weak If-None-Match matching and
obsolete HTTP dates are supported. 304 has no body.

If-Range accepts an exact strong ETag. Weak tags and dates yield the full 200:
a filesystem timestamp alone does not establish a strong validator. Unsatisfiable
simple ranges return 416 and bytes */length. Malformed, multiple and unsupported
ranges are ignored. Byte ranges refer to the selected encoded payload. Cold loads
and direct file reads remain synchronous. No transport optimization is introduced.

<a id="method-register-routes"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> register_routes

```cpp
template <typename Rty>
void register_routes(Rty& routes);
```

**Template parameters:** Rty supports controller route registration.

**Parameters:** routes receives GET/HEAD wildcard routes and optional exact mount redirect routes. Literal/parameterized routes keep their precedence. Other methods use existing 405/Allow handling.

**Returns:** void.

<a id="invalidate"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> invalidate

```cpp
void invalidate(std::string_view relative) const;
```

**Parameters:** relative is a decoded root-relative filename without leading slash.

**Returns:** void. Invalidates the filename and .br/.gz siblings. In-flight loads cannot republish; existing readers stay valid. No-op without a cache.

<a id="clear-cache"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> clear_cache

```cpp
void clear_cache() const;
```

**Returns:** void. Clears the entire supplied cache, including other mounts sharing it. No-op without a cache. Applications may keep the supplied cache pointer and call clear() when the router owns the controller.
