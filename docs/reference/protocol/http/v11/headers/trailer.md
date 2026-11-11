<a id="trailer"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/trailer/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/trailer/title.svg" alt="trailer">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/trailer/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/trailer/title-dark.svg" alt="trailer">
</a>

**Header:** [include/protocol/http/v11/headers/trailer.h](../../../../../../include/protocol/http/v11/headers/trailer.h)

HTTP/1.1 `trailer` parsing and interpretation.

<a id="api"></a>
<a id="type-trailer"></a>

## trailer

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## check

```cpp
static bool check(std::string_view sv, parsed_token_list& out);
```

Appends comma-separated field-name tokens.

Output lists are appended to, not cleared. Parsed views borrow the input; keep its backing bytes alive and unchanged. Failure may leave earlier elements appended, and allocation failures may throw. Use a fresh output for a new field. Empty comma elements are skipped; field-level whitespace trimming is the caller's responsibility.

<a id="method-interpret"></a>

## interpret

```cpp
static constexpr verdict interpret(const parsed_token_list& token_list,
                                     v11::connection& conn, const policies&);
```

Appends the names to `trailer_names` and accepts. It does not check whether names are forbidden or whether trailers actually arrive.
