<a id="request"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/request/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/request/title.svg" alt="request">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/request/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/request/title-dark.svg" alt="request">
</a>

**Header:** [include/protocol/http/v11/request.h](../../../../../include/protocol/http/v11/request.h)

Provides the http 1.1 request implementation.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11

[request](#type-request)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::request

[from](#method-from) ·
[get_method](#method-get-method) ·
[get_target](#method-get-target) ·
[get_absolute_path](#method-get-absolute-path) ·
[get_header(i)](#method-get-header) ·
[get_header(name)](#method-get-header-2) ·
[exist_header](#method-exist-header) ·
[get_headers_length](#method-get-headers-length) ·
[get_query_parameter(i)](#method-get-query-parameter) ·
[get_query_parameter(name)](#method-get-query-parameter-2) ·
[get_query_parameters_length](#method-get-query-parameters-length) ·
[has_host](#method-has-host) ·
[get_host](#method-get-host) ·
[get_host_port](#method-get-host-port) ·
[get_host_type](#method-get-host-type) ·
[has_target_authority](#method-has-target-authority) ·
[get_target_authority_host](#method-get-target-authority-host) ·
[get_target_authority_port](#method-get-target-authority-port) ·
[get_target_authority_type](#method-get-target-authority-type) ·
[get_body_reader](#method-get-body-reader) ·
[has_body_reader](#method-has-body-reader) ·
[wants_connection_close](#method-wants-connection-close) ·
[get_cookie](#method-get-cookie) ·
[get_cookies](#method-get-cookies) ·
[borrows_bytes](#method-borrows-bytes) ·
[make_owned](#method-make-owned) ·
[rebase_bytes](#method-rebase-bytes) ·
[body_is_stored](#method-body-is-stored) ·
[body_wire_size](#method-body-wire-size) ·
[begin_body](#method-begin-body) ·
[append_body](#method-append-body) ·
[finish_body](#method-finish-body)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-request"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> request

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
class request { ... };
```

Provides the http 1.1 request implementation.

**Source:** [include/protocol/http/v11/request.h:133](../../../../../include/protocol/http/v11/request.h#L133)

<a id="method-from"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> from

**METHOD** · *protocol::http::v11::request*

```cpp
static request from(
      std::string_view method, std::string_view abs_path, target target_form,
      std::vector<header_view> headers,
      std::vector<query_parameter_view> query_parameters,
      std::optional<std::string_view> host,
      std::optional<std::string_view> port,
      std::optional<helpers::host_type> type,
      std::optional<std::string_view> target_authority_host,
      std::optional<std::string_view> target_authority_port,
      std::optional<helpers::host_type> target_authority_type,
      bool wants_connection_close, std::optional<body::reader> body_reader,
      bool borrow_bytes = false);
```

Constructs the value from the supplied input.

**Parameters:**

- **method:** HTTP method to match. Declaration: `std::string_view method`.
- **abs_path:** Absolute request path. Declaration: `std::string_view abs_path`.
- **target_form:** Request-target form. Declaration: `target target_form`.
- **headers:** Request header collection. Declaration: `std::vector<header_view> headers`.
- **query_parameters:** Parsed query parameters. Declaration: `std::vector<query_parameter_view> query_parameters`.
- **host:** Host value, when available. Declaration: `std::optional<std::string_view> host`.
- **port:** Network port value. Declaration: `std::optional<std::string_view> port`.
- **type:** Type or classification to store or inspect. Declaration: `std::optional<helpers::host_type> type`.
- **target_authority_host:** Host in the target authority, when available. Declaration: `std::optional<std::string_view> target_authority_host`.
- **target_authority_port:** Port in the target authority, when available. Declaration: `std::optional<std::string_view> target_authority_port`.
- **target_authority_type:** Host type in the target authority, when available. Declaration: `std::optional<helpers::host_type> target_authority_type`.
- **wants_connection_close:** Whether the protocol wants to close the connection. Declaration: `bool wants_connection_close`.
- **body_reader:** Optional body reader. Declaration: `std::optional<body::reader> body_reader`.
- **borrow_bytes:** Whether to borrow the supplied request bytes. Declaration: `bool borrow_bytes = false`.

**Returns:** A value of type `request`.

**Source:** [include/protocol/http/v11/request.h:144](../../../../../include/protocol/http/v11/request.h#L144)

<a id="method-get-method"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_method

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_method() const;
```

Returns method.

**Returns:** The HTTP method (`std::string_view`).

**Source:** [include/protocol/http/v11/request.h:170](../../../../../include/protocol/http/v11/request.h#L170)

<a id="method-get-target"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_target

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_target() const;
```

Returns target.

**Returns:** The request-target form (`target`).

**Source:** [include/protocol/http/v11/request.h:174](../../../../../include/protocol/http/v11/request.h#L174)

<a id="method-get-absolute-path"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_absolute_path

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_absolute_path() const;
```

Returns absolute path.

**Returns:** The decoded absolute path (`std::string_view`).

**Source:** [include/protocol/http/v11/request.h:178](../../../../../include/protocol/http/v11/request.h#L178)

<a id="method-get-header"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_header

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_header(std::size_t i) const;
```

Returns header.

**Parameters:**

- **i:** Zero-based index of the requested element. Declaration: `std::size_t i`.

**Returns:** The matching header (`header_view`). The index overload accesses the indexed entry; the name overload searches case-insensitively.

**Source:** [include/protocol/http/v11/request.h:182](../../../../../include/protocol/http/v11/request.h#L182)

<a id="method-get-header-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_header

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_header(std::string_view name) const;
```

Returns header.

**Parameters:**

- **name:** Name to match or store. Declaration: `std::string_view name`.

**Returns:** The matching header (`header_view`). The index overload accesses the indexed entry; the name overload searches case-insensitively.

**Errors:** Throws `std::out_of_range` when the named header is absent.

**Source:** [include/protocol/http/v11/request.h:186](../../../../../include/protocol/http/v11/request.h#L186)

<a id="method-exist-header"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> exist_header

**METHOD** · *protocol::http::v11::request*

```cpp
auto exist_header(std::string_view name) const;
```

Checks whether the named HTTP header exists.

**Parameters:**

- **name:** Name to match or store. Declaration: `std::string_view name`.

**Returns:** `true` if the named header exists; otherwise `false`.

**Source:** [include/protocol/http/v11/request.h:195](../../../../../include/protocol/http/v11/request.h#L195)

<a id="method-get-headers-length"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_headers_length

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_headers_length() const;
```

Returns headers length.

**Returns:** The number of headers (`std::size_t`).

**Source:** [include/protocol/http/v11/request.h:204](../../../../../include/protocol/http/v11/request.h#L204)

<a id="method-get-query-parameter"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_query_parameter

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_query_parameter(std::size_t i) const;
```

Returns query parameter.

**Parameters:**

- **i:** Zero-based index of the requested element. Declaration: `std::size_t i`.

**Returns:** The indexed query parameter (`query_parameter_view`).

**Source:** [include/protocol/http/v11/request.h:208](../../../../../include/protocol/http/v11/request.h#L208)

<a id="method-get-query-parameter-2"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_query_parameter

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_query_parameter(std::string_view name) const
      -> std::optional<query_parameter_view>;
```

Returns query parameter.

**Parameters:**

- **name:** Name to match or store. Declaration: `std::string_view name`.

**Returns:** A `std::optional<query_parameter_view>` containing the result when present; otherwise `std::nullopt`.

**Source:** [include/protocol/http/v11/request.h:212](../../../../../include/protocol/http/v11/request.h#L212)

<a id="method-get-query-parameters-length"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_query_parameters_length

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_query_parameters_length() const;
```

Returns query parameters length.

**Returns:** The number of query parameters (`std::size_t`).

**Source:** [include/protocol/http/v11/request.h:222](../../../../../include/protocol/http/v11/request.h#L222)

<a id="method-has-host"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> has_host

**METHOD** · *protocol::http::v11::request*

```cpp
auto has_host() const;
```

Reports whether host applies.

**Returns:** `true` if a host value is present.

**Source:** [include/protocol/http/v11/request.h:226](../../../../../include/protocol/http/v11/request.h#L226)

<a id="method-get-host"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_host

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_host() const;
```

Returns host.

**Returns:** The host value (`std::string_view`).

**Source:** [include/protocol/http/v11/request.h:230](../../../../../include/protocol/http/v11/request.h#L230)

<a id="method-get-host-port"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_host_port

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_host_port() const;
```

Returns host port.

**Returns:** The host port (`std::string_view`).

**Source:** [include/protocol/http/v11/request.h:234](../../../../../include/protocol/http/v11/request.h#L234)

<a id="method-get-host-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_host_type

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_host_type() const;
```

Returns host type.

**Returns:** The host type (`helpers::host_type`).

**Source:** [include/protocol/http/v11/request.h:238](../../../../../include/protocol/http/v11/request.h#L238)

<a id="method-has-target-authority"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> has_target_authority

**METHOD** · *protocol::http::v11::request*

```cpp
auto has_target_authority() const;
```

Reports whether target authority applies.

**Returns:** `true` if the target authority contains a host.

**Source:** [include/protocol/http/v11/request.h:242](../../../../../include/protocol/http/v11/request.h#L242)

<a id="method-get-target-authority-host"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_target_authority_host

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_target_authority_host() const;
```

Returns target authority host.

**Returns:** The target authority host (`std::string_view`).

**Source:** [include/protocol/http/v11/request.h:246](../../../../../include/protocol/http/v11/request.h#L246)

<a id="method-get-target-authority-port"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_target_authority_port

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_target_authority_port() const;
```

Returns target authority port.

**Returns:** The target authority port (`std::string_view`).

**Source:** [include/protocol/http/v11/request.h:250](../../../../../include/protocol/http/v11/request.h#L250)

<a id="method-get-target-authority-type"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_target_authority_type

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_target_authority_type() const;
```

Returns target authority type.

**Returns:** The target authority host type (`helpers::host_type`).

**Source:** [include/protocol/http/v11/request.h:254](../../../../../include/protocol/http/v11/request.h#L254)

<a id="method-get-body-reader"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_body_reader

**METHOD** · *protocol::http::v11::request*

```cpp
auto get_body_reader() const;
```

Returns a non-owning pointer to this request's body reader. Reading through
the pointer advances the reader. Use it while the request is alive; do not
delete it or keep it after the request's lifetime. It can be passed directly
to [`response::set_body(body::reader*)`](response.md#method-set-body-2), which
reads it during that call.

**Returns:** A borrowed `body::reader*` when a reader exists; otherwise
`nullptr`.

**Source:** [include/protocol/http/v11/request.h:258](../../../../../include/protocol/http/v11/request.h#L258)

<a id="method-has-body-reader"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> has_body_reader

**METHOD** · *protocol::http::v11::request*

```cpp
auto has_body_reader() const;
```

Reports whether body reader applies.

**Returns:** `true` if a body reader exists.

**Source:** [include/protocol/http/v11/request.h:264](../../../../../include/protocol/http/v11/request.h#L264)

<a id="method-wants-connection-close"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> wants_connection_close

**METHOD** · *protocol::http::v11::request*

```cpp
auto wants_connection_close() const;
```

Reports whether the connection should close.

**Returns:** `true` if the protocol decided to close the connection.

**Source:** [include/protocol/http/v11/request.h:268](../../../../../include/protocol/http/v11/request.h#L268)

<a id="method-get-cookie"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_cookie

**METHOD** · *protocol::http::v11::request*

```cpp
std::optional<std::string_view> get_cookie(std::string_view name) const;
```

Looks up a single cookie-pair by name in the (unparsed) Cookie header, if present. Parsing is done on demand; nothing is cached, mirroring how headers_ and query_parameters_ are already accessed by linear scan.

**Parameters:**

- **name:** Name to match or store. Declaration: `std::string_view name`.

**Returns:** A `std::optional<std::string_view>` containing the result when present; otherwise `std::nullopt`.

**Source:** [include/protocol/http/v11/request.h:276](../../../../../include/protocol/http/v11/request.h#L276)

<a id="method-get-cookies"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> get_cookies

**METHOD** · *protocol::http::v11::request*

```cpp
std::vector<std::pair<std::string_view, std::string_view>> get_cookies()
      const;
```

Returns every cookie-pair present in the (unparsed) Cookie header, or an empty vector when the header is absent.

**Returns:** A value of type `std::vector<std::pair<std::string_view, std::string_view>>`.

**Source:** [include/protocol/http/v11/request.h:293](../../../../../include/protocol/http/v11/request.h#L293)

<a id="method-borrows-bytes"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> borrows_bytes

**METHOD** · *protocol::http::v11::request*

```cpp
bool borrows_bytes() const noexcept;
```

Async execution must own its bytes before calling user code.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/request.h:306](../../../../../include/protocol/http/v11/request.h#L306)

<a id="method-make-owned"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> make_owned

**METHOD** · *protocol::http::v11::request*

```cpp
void make_owned();
```

Creates owned.

**Returns:** No value (`void`).

**Errors:** Throws `std::length_error` if the combined request fields exceed `std::size_t`; allocation errors may propagate.

**Source:** [include/protocol/http/v11/request.h:307](../../../../../include/protocol/http/v11/request.h#L307)

<a id="method-rebase-bytes"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> rebase_bytes

**METHOD** · *protocol::http::v11::request*

```cpp
void rebase_bytes(std::uintptr_t previous, const char* current) noexcept;
```

Rebases borrowed views onto a new byte buffer.

**Parameters:**

- **previous:** Address of the previous byte buffer. Declaration: `std::uintptr_t previous`.
- **current:** Address of the replacement byte buffer. Declaration: `const char* current`.

**Returns:** No value (`void`).

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/request.h:350](../../../../../include/protocol/http/v11/request.h#L350)

<a id="method-body-is-stored"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> body_is_stored

**METHOD** · *protocol::http::v11::request*

```cpp
bool body_is_stored() const noexcept;
```

Reports whether body storage is present.

**Returns:** A boolean result; see the operation description for its meaning.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/request.h:367](../../../../../include/protocol/http/v11/request.h#L367)

<a id="method-body-wire-size"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> body_wire_size

**METHOD** · *protocol::http::v11::request*

```cpp
std::size_t body_wire_size() const noexcept;
```

Returns the body size on the wire, in bytes.

**Returns:** The body wire size count or size.

**Errors:** Declared `noexcept`.

**Source:** [include/protocol/http/v11/request.h:368](../../../../../include/protocol/http/v11/request.h#L368)

<a id="method-begin-body"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> begin_body

**METHOD** · *protocol::http::v11::request*

```cpp
bool begin_body(std::size_t memory_threshold, std::size_t declared_size);
```

Inline wire bytes stay in the decoder; larger bodies use existing storage.

**Parameters:**

- **memory_threshold:** Maximum body size retained in memory, in bytes. Declaration: `std::size_t memory_threshold`.
- **declared_size:** Size or length limit for the named data. Declaration: `std::size_t declared_size`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/request.h:371](../../../../../include/protocol/http/v11/request.h#L371)

<a id="method-append-body"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> append_body

**METHOD** · *protocol::http::v11::request*

```cpp
bool append_body(std::string_view bytes);
```

Appends body.

**Parameters:**

- **bytes:** Byte sequence or byte count for this operation. Declaration: `std::string_view bytes`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/request.h:382](../../../../../include/protocol/http/v11/request.h#L382)

<a id="method-finish-body"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> finish_body

**METHOD** · *protocol::http::v11::request*

```cpp
bool finish_body(bool chunked, std::size_t content_length,
                   std::size_t extension_limit, std::size_t trailer_limit);
```

Finalizes body storage and prepares its reader.

**Parameters:**

- **chunked:** Whether the body uses chunked transfer encoding. Declaration: `bool chunked`.
- **content_length:** Size or length limit for the named data. Declaration: `std::size_t content_length`.
- **extension_limit:** Maximum chunk-extension size in bytes. Declaration: `std::size_t extension_limit`.
- **trailer_limit:** Maximum trailer size in bytes. Declaration: `std::size_t trailer_limit`.

**Returns:** A boolean result; see the operation description for its meaning.

**Source:** [include/protocol/http/v11/request.h:400](../../../../../include/protocol/http/v11/request.h#L400)
