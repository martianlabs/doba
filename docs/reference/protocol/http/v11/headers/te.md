<a id="te"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/te/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/te/title.svg" alt="te">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/te/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/te/title-dark.svg" alt="te">
</a>

**Header:** [include/protocol/http/v11/headers/te.h](../../../../../../include/protocol/http/v11/headers/te.h)

HTTP/1.1 `te` parsing and interpretation.

<a id="api"></a>
<a id="type-te"></a>

## te

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## check

```cpp
static bool check(std::string_view sv, parsed_parameter_list& out);
```

Appends coding elements with parameters. A case-insensitive `q` weight must be unique and last, with no whitespace around `=`; other parameters allow such whitespace. Empty semicolon slots are rejected.

Output lists are appended to, not cleared. Parsed views borrow the input; keep its backing bytes alive and unchanged. Failure may leave earlier elements appended, and allocation failures may throw. Use a fresh output for a new field. Empty comma elements are skipped; field-level whitespace trimming is the caller's responsibility.

<a id="method-interpret"></a>

## interpret

```cpp
static constexpr verdict interpret(
      const parsed_parameter_list& parameters_list, v11::connection& conn,
      const policies&);
```

Sets `accepts_trailers` for the `trailers` token and appends other coding names to `te_codings`. Parameter/weight text is not retained there. Always accepts.
