<a id="forwarded"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/forwarded/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/forwarded/title.svg" alt="forwarded">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/forwarded/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/forwarded/title-dark.svg" alt="forwarded">
</a>

**Header:** [include/protocol/http/v11/headers/forwarded.h](../../../../../../include/protocol/http/v11/headers/forwarded.h)

HTTP/1.1 `forwarded` parsing and interpretation.

<a id="api"></a>
<a id="type-forwarded"></a>

## forwarded

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## check

```cpp
static bool check(std::string_view sv, parsed_forwarded_list& out);
```

Appends comma-separated elements containing semicolon-separated `name=value` pairs. No whitespace is allowed around `=` or after semicolons; quoted values retain their quotes and escapes. Empty semicolon slots are skipped. Success requires `out.elements` to be nonempty, so use a fresh output for each field.

Output lists are appended to, not cleared. Parsed views borrow the input; keep its backing bytes alive and unchanged. Failure may leave earlier elements appended, and allocation failures may throw. Use a fresh output for a new field. Empty comma elements are skipped; field-level whitespace trimming is the caller's responsibility.

<a id="method-interpret"></a>

## interpret

```cpp
static constexpr verdict interpret(
      const parsed_forwarded_list& forwarded_list, v11::connection&,
      const policies& pol);
```

Rejects when this parsed list exceeds a nonzero `max_forwarding_hops`; otherwise accepts without changing the connection. It does not establish proxy trust.
