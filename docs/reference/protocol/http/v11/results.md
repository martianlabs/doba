<a id="results"></a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/results/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/results/title.svg" alt="results">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/v11/results/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/v11/results/title-dark.svg" alt="results">
</a>

**Header:** [include/protocol/http/v11/results.h](../../../../../include/protocol/http/v11/results.h)

HTTP/1.1 decoding result for the selected request and response types.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11

[deserialization_result](#type-deserialization-result) ·
[serialization_result](#type-serialization-result)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::deserialization_result

[code](#field-code) ·
[request](#field-request) ·
[response](#field-response)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::v11::serialization_result

[head](#field-head) ·
[body](#field-body) ·
[source](#field-source)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-deserialization-result"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> deserialization_result

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
template <typename RQty, typename RSty>
struct deserialization_result { ... };
```

HTTP/1.1 decoding result for the selected request and response types.

**Template parameters:**

- **RQty:** `typename RQty`.
- **RSty:** `typename RSty`.

**Source:** [include/protocol/http/v11/results.h:44](../../../../../include/protocol/http/v11/results.h#L44)

<a id="field-code"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> code

**FIELD** · *protocol::http::v11::deserialization_result*

```cpp
deserialization_status code = deserialization_status::kInvalidSource;
```

Stores the code value.

**Value:** `deserialization_status::kInvalidSource`

**Source:** [include/protocol/http/v11/results.h:60](../../../../../include/protocol/http/v11/results.h#L60)

<a id="field-request"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> request

**FIELD** · *protocol::http::v11::deserialization_result*

```cpp
std::optional<RQty> request;
```

Stores the request value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/results.h:61](../../../../../include/protocol/http/v11/results.h#L61)

<a id="field-response"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> response

**FIELD** · *protocol::http::v11::deserialization_result*

```cpp
std::optional<RSty> response;
```

Stores the response value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/results.h:62](../../../../../include/protocol/http/v11/results.h#L62)

<a id="type-serialization-result"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> serialization_result

**TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct serialization_result { ... };
```

Borrows head and body bytes and owns an optional source. The transport copies both views before the call returns.

**Source:** [include/protocol/http/v11/results.h:73](../../../../../include/protocol/http/v11/results.h#L73)

<a id="field-head"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> head

**FIELD** · *protocol::http::v11::serialization_result*

```cpp
std::string_view head;
```

Stores the head value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/results.h:77](../../../../../include/protocol/http/v11/results.h#L77)

<a id="field-body"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> body

**FIELD** · *protocol::http::v11::serialization_result*

```cpp
std::string_view body;
```

Stores the body value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/results.h:78](../../../../../include/protocol/http/v11/results.h#L78)

<a id="field-source"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> source

**FIELD** · *protocol::http::v11::serialization_result*

```cpp
std::unique_ptr<common::reader> source;
```

Stores the source value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/results.h:79](../../../../../include/protocol/http/v11/results.h#L79)
