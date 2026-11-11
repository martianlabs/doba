<a id="x-forwarded-for"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_for/title.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_for/title.svg" alt="x_forwarded_for">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_for/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_for/title-dark.svg" alt="x_forwarded_for">
</a>

**Header:** [include/protocol/http/v11/headers/x_forwarded_for.h](../../../../../../include/protocol/http/v11/headers/x_forwarded_for.h)

HTTP/1.1 `x-forwarded-for` parsing and interpretation.

<a id="api"></a>
<a id="type-x-forwarded-for"></a>

## x_forwarded_for

Static operations in `martianlabs::doba::protocol::http::v11::headers`.

<a id="method-check"></a>

## check

```cpp
static bool check(std::string_view sv, parsed_token_list& out);
```

Appends strict IPv4, unbracketed IPv6, bracketed IP literals, or exactly lowercase `unknown`. Hostnames and address/port forms are rejected.

Output lists are appended to, not cleared. Parsed views borrow the input; keep its backing bytes alive and unchanged. Failure may leave earlier elements appended, and allocation failures may throw. Use a fresh output for a new field. Empty comma elements are skipped; field-level whitespace trimming is the caller's responsibility.

<a id="method-interpret"></a>

## interpret

```cpp
static constexpr verdict interpret(const parsed_token_list& token_list,
                                     v11::connection&, const policies& pol);
```

Rejects when this parsed list exceeds a nonzero `max_forwarding_hops`; otherwise accepts without changing the connection. It does not establish proxy trust.
