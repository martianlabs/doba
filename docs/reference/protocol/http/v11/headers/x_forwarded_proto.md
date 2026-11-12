<a id="x-forwarded-proto"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_proto/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_proto/title.svg" alt="x_forwarded_proto">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_proto/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_proto/title-dark.svg" alt="x_forwarded_proto">
</a>

**Header:** [include/protocol/http/v11/headers/x_forwarded_proto.h](../../../../../../include/protocol/http/v11/headers/x_forwarded_proto.h)

HTTP/1.1 `x-forwarded-proto` parsing and interpretation.

<a id="api"></a>
<a id="type-x-forwarded-proto"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> x_forwarded_proto

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static bool check(std::string_view sv, parsed_token_list& out);
```

Appends comma-separated URI scheme names: an ASCII letter followed by letters, digits, `+`, `-`, or `.`.

Output lists are appended to, not cleared. Parsed views borrow the input; keep its backing bytes alive and unchanged. Failure may leave earlier elements appended, and allocation failures may throw. Use a fresh output for a new field. Empty comma elements are skipped; field-level whitespace trimming is the caller's responsibility.

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

```cpp
static constexpr verdict interpret(const parsed_token_list&, v11::connection&,
                                     const policies&);
```

Always accepts without changing the connection. It does not change whether the connection uses TLS.
