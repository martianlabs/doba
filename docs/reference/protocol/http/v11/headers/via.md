<a id="via"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/via/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/via/title.svg" alt="via">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/via/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/via/title-dark.svg" alt="via">
</a>

**Header:** [include/protocol/http/v11/headers/via.h](../../../../../../include/protocol/http/v11/headers/via.h)

HTTP/1.1 `via` parsing and interpretation.

<a id="api"></a>
<a id="type-via"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> via

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static bool check(std::string_view sv, parsed_via_list& out);
```

Appends entries containing a protocol token (optionally `name/version`), required whitespace, a token recipient with optional decimal port, and an optional balanced comment. Commas inside comments are preserved. Bracketed IPv6 recipients are not supported by this parser. Empty lists are accepted.

Output lists are appended to, not cleared. Parsed views borrow the input; keep its backing bytes alive and unchanged. Failure may leave earlier elements appended, and allocation failures may throw. Use a fresh output for a new field. Empty comma elements are skipped; field-level whitespace trimming is the caller's responsibility.

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

```cpp
static constexpr verdict interpret(const parsed_via_list& parsed_list,
                                     v11::connection&,
                                     const policies& policies);
```

Rejects when this parsed list exceeds a nonzero `max_forwarding_hops`; otherwise accepts without changing the connection.
