<a id="x-forwarded-for"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_for/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_for/index.svg" alt="x_forwarded_for">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_for/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/x_forwarded_for/index-dark.svg" alt="x_forwarded_for">
</a>

**Header:** [include/protocol/http/v11/headers/x_forwarded_for.h](../../../../../../include/protocol/http/v11/headers/x_forwarded_for.h)

|                                                           x-forwarded-for | | De-facto X-Forwarded-For                                                  | | The "X-Forwarded-For" header field is a de-facto request header used by   | | proxies, reverse proxies, load balancers, and gateways to identify the    | | originating client IP address and th

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers

[x_forwarded_for](#type-x-forwarded-for)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::x_forwarded_for

[check](#method-check) ·
[interpret](#method-interpret)

<a id="type-x-forwarded-for"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> x_forwarded_for

**TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class x_forwarded_for { ... };
```

|                                                           x-forwarded-for | | De-facto X-Forwarded-For                                                  | | The "X-Forwarded-For" header field is a de-facto request header used by   | | proxies, reverse proxies, load balancers, and gateways to identify the    | | originating client IP address and th

**Source:** [include/protocol/http/v11/headers/x_forwarded_for.h:67](../../../../../../include/protocol/http/v11/headers/x_forwarded_for.h#L67)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::v11::headers::x_forwarded_for*

```cpp
static bool check(std::string_view sv, parsed_token_list& out);
```

Checks whether a value follows the HTTP x-forwarded-for header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_token_list& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/x_forwarded_for.h:72](../../../../../../include/protocol/http/v11/headers/x_forwarded_for.h#L72)

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

**METHOD** · *protocol::http::v11::headers::x_forwarded_for*

```cpp
static constexpr verdict interpret(const parsed_token_list& token_list,
                                     v11::connection&, const policies& pol);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **token_list:** Parsed token list to process. Declaration: `const parsed_token_list& token_list`.
- **connection:** Connection being processed. Declaration: `v11::connection&`.
- **pol:** Policies controlling this operation. Declaration: `const policies& pol`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/x_forwarded_for.h:84](../../../../../../include/protocol/http/v11/headers/x_forwarded_for.h#L84)
