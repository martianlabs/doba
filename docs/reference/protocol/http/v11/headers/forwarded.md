<a id="forwarded"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/forwarded/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/forwarded/index.svg" alt="forwarded">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/forwarded/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/forwarded/index-dark.svg" alt="forwarded">
</a>

**Header:** [include/protocol/http/v11/headers/forwarded.h](../../../../../../include/protocol/http/v11/headers/forwarded.h)

|                                                                 forwarded | | RFC 7239 S4 Forwarded                                                     | | The "Forwarded" header field is an optional request header field used by  | | proxies to disclose information that is altered or lost when a request    | | is forwarded. It can carry, for exam

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers

[forwarded](#type-forwarded)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::forwarded

[check](#method-check) ·
[interpret](#method-interpret)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-forwarded"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> forwarded

**TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class forwarded { ... };
```

|                                                                 forwarded | | RFC 7239 S4 Forwarded                                                     | | The "Forwarded" header field is an optional request header field used by  | | proxies to disclose information that is altered or lost when a request    | | is forwarded. It can carry, for exam

**Source:** [include/protocol/http/v11/headers/forwarded.h:108](../../../../../../include/protocol/http/v11/headers/forwarded.h#L108)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::v11::headers::forwarded*

```cpp
static bool check(std::string_view sv, parsed_forwarded_list& out);
```

Checks whether a value follows the HTTP forwarded header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `parsed_forwarded_list& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/forwarded.h:113](../../../../../../include/protocol/http/v11/headers/forwarded.h#L113)

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

**METHOD** · *protocol::http::v11::headers::forwarded*

```cpp
static constexpr verdict interpret(
      const parsed_forwarded_list& forwarded_list, v11::connection&,
      const policies& pol);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **forwarded_list:** Parsed Forwarded header elements. Declaration: `const parsed_forwarded_list& forwarded_list`.
- **connection:** Connection being processed. Declaration: `v11::connection&`.
- **pol:** Policies controlling this operation. Declaration: `const policies& pol`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/forwarded.h:129](../../../../../../include/protocol/http/v11/headers/forwarded.h#L129)
