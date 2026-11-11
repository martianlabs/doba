<a id="content-length"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/content_length/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/content_length/title.svg" alt="content_length">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/content_length/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/content_length/title-dark.svg" alt="content_length">
</a>

**Header:** [include/protocol/http/v11/headers/content_length.h](../../../../../../include/protocol/http/v11/headers/content_length.h)

HTTP/1.1 `content-length` parsing and interpretation.

<a id="api"></a>
<a id="type-content-length"></a>

## content_length

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## check

```cpp
static constexpr bool check(std::string_view sv, std::size_t& out);
```

Parses a whole nonempty decimal number into `size_t`, rejecting overflow. Leading zeroes are accepted; `out` is unchanged on failure.

<a id="method-interpret"></a>

## interpret

```cpp
static constexpr verdict interpret(const std::size_t& len, v11::connection&,
                                     const policies& policies);
```

Rejects a length above a nonzero `max_request_body_size`; otherwise accepts. Does not change the connection.
