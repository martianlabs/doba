<a id="max-forwards"></a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/max_forwards/index.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/max_forwards/index.svg" alt="max_forwards">
</a>
<a href="../../../../../../resources/docs/reference/protocol/http/v11/headers/max_forwards/index-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/protocol/http/v11/headers/max_forwards/index-dark.svg" alt="max_forwards">
</a>

**Header:** [include/protocol/http/v11/headers/max_forwards.h](../../../../../../include/protocol/http/v11/headers/max_forwards.h)

|                                                              max-forwards | | RFC 9110 S7.6.2 Max-Forwards                                              | | The "Max-Forwards" header field provides a mechanism with the TRACE and   | | OPTIONS request methods to limit the number of times that a request is    | | forwarded by proxies.

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers

[max_forwards](#type-max-forwards)

### <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::headers::max_forwards

[check](#method-check) ·
[interpret](#method-interpret)

<a id="api-details"></a>
<a href="../../../../../../resources/docs/reference/api-details.svg#gh-light-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details.svg" alt="API details">
</a>
<a href="../../../../../../resources/docs/reference/api-details-dark.svg#gh-dark-mode-only">
  <img src="../../../../../../resources/docs/reference/api-details-dark.svg" alt="API details">
</a>

<a id="type-max-forwards"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> max_forwards

**TYPE** · *protocol::http::v11::headers*

**Declaration synopsis:**

```cpp
class max_forwards { ... };
```

|                                                              max-forwards | | RFC 9110 S7.6.2 Max-Forwards                                              | | The "Max-Forwards" header field provides a mechanism with the TRACE and   | | OPTIONS request methods to limit the number of times that a request is    | | forwarded by proxies.

**Source:** [include/protocol/http/v11/headers/max_forwards.h:64](../../../../../../include/protocol/http/v11/headers/max_forwards.h#L64)

<a id="method-check"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> check

**METHOD** · *protocol::http::v11::headers::max_forwards*

```cpp
static constexpr bool check(std::string_view sv, std::size_t& out);
```

Checks whether a value follows the HTTP max-forwards header syntax.

**Parameters:**

- **sv:** Text to check or parse. Declaration: `std::string_view sv`.
- **out:** Output parameter that receives the result. Declaration: `std::size_t& out`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/headers/max_forwards.h:69](../../../../../../include/protocol/http/v11/headers/max_forwards.h#L69)

<a id="method-interpret"></a>

## <a href="../../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> interpret

**METHOD** · *protocol::http::v11::headers::max_forwards*

```cpp
static constexpr verdict interpret(const std::size_t&,
                                     v11::connection&, const policies&);
```

Interprets the parsed header value for HTTP/1.1 processing.

**Parameters:**

- **size_t:** Size value. Declaration: `const std::size_t&`.
- **connection:** Connection being processed. Declaration: `v11::connection&`.
- **policies:** Configuration policies for this component. Declaration: `const policies&`.

**Returns:** A `verdict` describing the result of applying this header or rule.

**Source:** [include/protocol/http/v11/headers/max_forwards.h:78](../../../../../../include/protocol/http/v11/headers/max_forwards.h#L78)
