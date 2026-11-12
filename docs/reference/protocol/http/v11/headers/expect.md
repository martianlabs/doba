<a id="expect"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/expect/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/expect/title.svg" alt="expect">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/expect/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/expect/title-dark.svg" alt="expect">
</a>

**Header:** [include/protocol/http/v11/headers/expect.h](../../../../../../include/protocol/http/v11/headers/expect.h)

HTTP/1.1 `expect` parsing and interpretation.

<a id="api"></a>
<a id="type-expect"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> expect

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

```cpp
static bool check(std::string_view sv, parsed_parameter_list& out);
```

Appends expectation tokens, optionally followed by `=value` and semicolon parameters. Values may be tokens or quoted strings.

Output lists are appended to, not cleared. Parsed views borrow the input; keep its backing bytes alive and unchanged. Failure may leave earlier elements appended, and allocation failures may throw. Use a fresh output for a new field. Empty comma elements are skipped; field-level whitespace trimming is the caller's responsibility.

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

```cpp
static constexpr verdict interpret(
      const parsed_parameter_list& parameters_list, v11::connection& conn,
      const policies&);
```

Accepts only bare, case-insensitive `100-continue` elements and sets `expects_continue`. Other expectations reject; changes made before a rejected element remain.
