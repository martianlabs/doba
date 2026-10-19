<a id="method-names-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/common/method_names/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/common/method_names/index.svg" alt="Index for method_names.h">
  </picture>
</h1>

**Header:** [include/protocol/http/common/method_names.h](../../../../../include/protocol/http/common/method_names.h)

Provides the http 1.1 method names specification.

**protocol::http**

- [method_names](#type-method-names) · *type*

**protocol::http::method_names**

- [kGet](#constant-kget) · *constant*
- [kHead](#constant-khead) · *constant*
- [kPost](#constant-kpost) · *constant*
- [kPut](#constant-kput) · *constant*
- [kDelete](#constant-kdelete) · *constant*
- [kConnect](#constant-kconnect) · *constant*
- [kOptions](#constant-koptions) · *constant*
- [kTrace](#constant-ktrace) · *constant*

<a id="type-method-names"></a>

## method_names

🟠 **TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
struct method_names { ... };
```

Provides the http 1.1 method names specification.

**Source:** [include/protocol/http/common/method_names.h:36](../../../../../include/protocol/http/common/method_names.h#L36)

<a id="constant-kget"></a>

## kGet

🟠 **CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kGet[] = "GET";
```

Method  | Description GET     | Transfer a current representation of the target resource. HEAD    | Same as GET, but do not transfer the response content. POST    | Perform resource-specific processing on the request content. PUT     | Replace all current representations of the target resource | with the request content. DELETE  | Remove all

**Value:** `"GET"`

**Source:** [include/protocol/http/common/method_names.h:55](../../../../../include/protocol/http/common/method_names.h#L55)

<a id="constant-khead"></a>

## kHead

🟠 **CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kHead[] = "HEAD";
```

Defines the head constant.

**Value:** `"HEAD"`

**Source:** [include/protocol/http/common/method_names.h:56](../../../../../include/protocol/http/common/method_names.h#L56)

<a id="constant-kpost"></a>

## kPost

🟠 **CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kPost[] = "POST";
```

Defines the post constant.

**Value:** `"POST"`

**Source:** [include/protocol/http/common/method_names.h:57](../../../../../include/protocol/http/common/method_names.h#L57)

<a id="constant-kput"></a>

## kPut

🟠 **CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kPut[] = "PUT";
```

Defines the put constant.

**Value:** `"PUT"`

**Source:** [include/protocol/http/common/method_names.h:58](../../../../../include/protocol/http/common/method_names.h#L58)

<a id="constant-kdelete"></a>

## kDelete

🟠 **CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kDelete[] = "DELETE";
```

Defines the delete constant.

**Value:** `"DELETE"`

**Source:** [include/protocol/http/common/method_names.h:59](../../../../../include/protocol/http/common/method_names.h#L59)

<a id="constant-kconnect"></a>

## kConnect

🟠 **CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kConnect[] = "CONNECT";
```

Defines the connect constant.

**Value:** `"CONNECT"`

**Source:** [include/protocol/http/common/method_names.h:60](../../../../../include/protocol/http/common/method_names.h#L60)

<a id="constant-koptions"></a>

## kOptions

🟠 **CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kOptions[] = "OPTIONS";
```

Defines the options constant.

**Value:** `"OPTIONS"`

**Source:** [include/protocol/http/common/method_names.h:61](../../../../../include/protocol/http/common/method_names.h#L61)

<a id="constant-ktrace"></a>

## kTrace

🟠 **CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kTrace[] = "TRACE";
```

Defines the trace constant.

**Value:** `"TRACE"`

**Source:** [include/protocol/http/common/method_names.h:62](../../../../../include/protocol/http/common/method_names.h#L62)
