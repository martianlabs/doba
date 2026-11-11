<a id="connection"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/connection/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/connection/title.svg" alt="connection">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/connection/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/connection/title-dark.svg" alt="connection">
</a>

**Header:** [include/protocol/http/v11/headers/connection.h](../../../../../../include/protocol/http/v11/headers/connection.h)

HTTP/1.1 `connection` parsing and interpretation.

<a id="api"></a>
<a id="type-connection"></a>

## connection

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## check

```cpp
static bool check(std::string_view sv, parsed_token_list& out);
```

Appends comma-separated option tokens to `out.elements`.

Output lists are appended to, not cleared. Parsed views borrow the input; keep its backing bytes alive and unchanged. Failure may leave earlier elements appended, and allocation failures may throw. Use a fresh output for a new field. Empty comma elements are skipped; field-level whitespace trimming is the caller's responsibility.

<a id="method-interpret"></a>

## interpret

```cpp
static constexpr verdict interpret(const parsed_token_list& token_list,
                                     v11::connection& connection_out,
                                     const policies&);
```

Appends options to the connection. A case-insensitive `close` sets `close_requested=true` and `persistent=false`; other options do not restore persistence.
