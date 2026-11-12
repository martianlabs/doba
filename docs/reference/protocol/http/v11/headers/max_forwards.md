<a id="max-forwards"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/max_forwards/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/max_forwards/title.svg" alt="max_forwards">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/max_forwards/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/max_forwards/title-dark.svg" alt="max_forwards">
</a>

**Header:** [include/protocol/http/v11/headers/max_forwards.h](../../../../../../include/protocol/http/v11/headers/max_forwards.h)

HTTP/1.1 `max-forwards` parsing and interpretation.

<a id="api"></a>
<a id="type-max-forwards"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_forwards

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static constexpr bool check(std::string_view sv, std::size_t& out);
```

Parses a whole nonempty decimal number into `size_t`, rejecting overflow. Leading zeroes are accepted; `out` is unchanged on failure.

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

```cpp
static constexpr verdict interpret(const std::size_t&,
                                     v11::connection&, const policies&);
```

Always accepts. It does not decrement the value or implement proxy forwarding.
