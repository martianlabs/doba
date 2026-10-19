<a id="results-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/v11/results/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/results/index.svg" alt="Index for results.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/results.h](../../../../../include/protocol/http/v11/results.h)

HTTP/1.1 decoding result for the selected request and response types.

**protocol::http::v11**

- [deserialization_result](#type-deserialization-result) · *type*

**protocol::http::v11::deserialization_result**

- [deserialization_result()](#constructor-deserialization-result) · *constructor*
- [deserialization_result(code)](#constructor-deserialization-result-2) · *constructor*
- [deserialization_result(deserialization_result)](#constructor-deserialization-result-3) · *constructor*
- [deserialization_result(deserialization_result)](#constructor-deserialization-result-4) · *constructor*
- [operator=(deserialization_result)](#operator-operator) · *operator*
- [operator=(deserialization_result)](#operator-operator-2) · *operator*
- [code](#field-code) · *field*
- [request](#field-request) · *field*
- [response](#field-response) · *field*

**protocol::http::v11**

- [serialization_result](#type-serialization-result) · *type*

**protocol::http::v11::serialization_result**

- [head](#field-head) · *field*
- [body](#field-body) · *field*
- [source](#field-source) · *field*

<a id="type-deserialization-result"></a>

## deserialization_result

🟠 **TYPE** · *protocol::http::v11*

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

<a id="constructor-deserialization-result"></a>

## deserialization_result

🟠 **CONSTRUCTOR** · *protocol::http::v11::deserialization_result*

```cpp
deserialization_result();
```

Constructs a deserialization_result value.

**Source:** [include/protocol/http/v11/results.h:48](../../../../../include/protocol/http/v11/results.h#L48)

<a id="constructor-deserialization-result-2"></a>

## deserialization_result

🟠 **CONSTRUCTOR** · *protocol::http::v11::deserialization_result*

```cpp
deserialization_result(deserialization_status code);
```

Constructs a deserialization_result value.

**Parameters:**

- **code:** Initial value supplied to the constructor. Declaration: `deserialization_status code`.

**Source:** [include/protocol/http/v11/results.h:49](../../../../../include/protocol/http/v11/results.h#L49)

<a id="constructor-deserialization-result-3"></a>

## deserialization_result

🟠 **CONSTRUCTOR** · *protocol::http::v11::deserialization_result*

```cpp
deserialization_result(const deserialization_result&) = default;
```

Constructs a deserialization_result value.

**Parameters:**

- **deserialization_result:** Source object to copy or move. Declaration: `const deserialization_result&`.

**Source:** [include/protocol/http/v11/results.h:50](../../../../../include/protocol/http/v11/results.h#L50)

<a id="constructor-deserialization-result-4"></a>

## deserialization_result

🟠 **CONSTRUCTOR** · *protocol::http::v11::deserialization_result*

```cpp
deserialization_result(deserialization_result&&) = default;
```

Constructs a deserialization_result value.

**Parameters:**

- **deserialization_result:** Source object to copy or move. Declaration: `deserialization_result&&`.

**Source:** [include/protocol/http/v11/results.h:51](../../../../../include/protocol/http/v11/results.h#L51)

<a id="operator-operator"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::v11::deserialization_result*

```cpp
deserialization_result& operator=(const deserialization_result&) = default;
```

Uses the compiler-generated operator= operation.

**Parameters:**

- **deserialization_result:** Source object to copy or move. Declaration: `const deserialization_result&`.

**Returns:** A reference to the updated object.

**Source:** [include/protocol/http/v11/results.h:55](../../../../../include/protocol/http/v11/results.h#L55)

<a id="operator-operator-2"></a>

## operator=

🟠 **OPERATOR** · *protocol::http::v11::deserialization_result*

```cpp
deserialization_result& operator=(deserialization_result&&) = default;
```

Uses the compiler-generated operator= operation.

**Parameters:**

- **deserialization_result:** Source object to copy or move. Declaration: `deserialization_result&&`.

**Returns:** A reference to the updated object.

**Source:** [include/protocol/http/v11/results.h:56](../../../../../include/protocol/http/v11/results.h#L56)

<a id="field-code"></a>

## code

🟠 **FIELD** · *protocol::http::v11::deserialization_result*

```cpp
deserialization_status code = deserialization_status::kInvalidSource;
```

Stores the code value.

**Value:** `deserialization_status::kInvalidSource`

**Source:** [include/protocol/http/v11/results.h:60](../../../../../include/protocol/http/v11/results.h#L60)

<a id="field-request"></a>

## request

🟠 **FIELD** · *protocol::http::v11::deserialization_result*

```cpp
std::optional<RQty> request;
```

Stores the request value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/results.h:61](../../../../../include/protocol/http/v11/results.h#L61)

<a id="field-response"></a>

## response

🟠 **FIELD** · *protocol::http::v11::deserialization_result*

```cpp
std::optional<RSty> response;
```

Stores the response value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/results.h:62](../../../../../include/protocol/http/v11/results.h#L62)

<a id="type-serialization-result"></a>

## serialization_result

🟠 **TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct serialization_result { ... };
```

Borrows head and body bytes and owns an optional source. The transport copies both views before the call returns.

**Source:** [include/protocol/http/v11/results.h:73](../../../../../include/protocol/http/v11/results.h#L73)

<a id="field-head"></a>

## head

🟠 **FIELD** · *protocol::http::v11::serialization_result*

```cpp
std::string_view head;
```

Stores the head value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/results.h:77](../../../../../include/protocol/http/v11/results.h#L77)

<a id="field-body"></a>

## body

🟠 **FIELD** · *protocol::http::v11::serialization_result*

```cpp
std::string_view body;
```

Stores the body value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/results.h:78](../../../../../include/protocol/http/v11/results.h#L78)

<a id="field-source"></a>

## source

🟠 **FIELD** · *protocol::http::v11::serialization_result*

```cpp
std::unique_ptr<common::reader> source;
```

Stores the source value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/results.h:79](../../../../../include/protocol/http/v11/results.h#L79)
