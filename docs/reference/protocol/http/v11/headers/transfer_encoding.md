<a id="transfer-encoding"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/transfer_encoding/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/transfer_encoding/title.svg" alt="transfer_encoding">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/transfer_encoding/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/transfer_encoding/title-dark.svg" alt="transfer_encoding">
</a>

**Header:** [include/protocol/http/v11/headers/transfer_encoding.h](../../../../../../include/protocol/http/v11/headers/transfer_encoding.h)

HTTP/1.1 `transfer-encoding` parsing and interpretation.

<a id="api"></a>
<a id="type-transfer-encoding"></a>

## transfer_encoding

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## check

```cpp
static bool check(std::string_view sv, parsed_parameter_list& out);
```

Appends coding elements with optional required, nonempty semicolon parameters. Whitespace around parameter `=` is allowed.

Output lists are appended to, not cleared. Parsed views borrow the input; keep its backing bytes alive and unchanged. Failure may leave earlier elements appended, and allocation failures may throw. Use a fresh output for a new field. Empty comma elements are skipped; field-level whitespace trimming is the caller's responsibility.

<a id="method-interpret"></a>

## interpret

```cpp
static constexpr verdict interpret(
      const parsed_parameter_list& parameters_list, v11::connection& conn,
      const policies& pol);
```

Rejects before appending when the combined coding count exceeds a nonzero `max_transfer_codings`. Otherwise appends coding names and sets `chunked` from the last name. If chunked is disabled, rejection occurs after these changes. Coding support and ordering are checked later by the decoder.
