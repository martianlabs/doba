<a id="method-names"></a>
<a href="../../../../../resources/docs/reference/protocol/http/common/method_names/title.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/method_names/title.svg" alt="method_names">
</a>
<a href="../../../../../resources/docs/reference/protocol/http/common/method_names/title-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/protocol/http/common/method_names/title-dark.svg" alt="method_names">
</a>

**Header:** [include/protocol/http/common/method_names.h](../../../../../include/protocol/http/common/method_names.h)

Provides the http 1.1 method names specification.

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http

[method_names](#type-method-names)

### <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> protocol::http::method_names

[kGet](#constant-kget) ·
[kHead](#constant-khead) ·
[kPost](#constant-kpost) ·
[kPut](#constant-kput) ·
[kDelete](#constant-kdelete) ·
[kConnect](#constant-kconnect) ·
[kOptions](#constant-koptions) ·
[kTrace](#constant-ktrace)

<a id="api"></a>
<a href="../../../../../resources/docs/reference/api.svg#gh-light-mode-only">
  <img src="../../../../../resources/docs/reference/api.svg" alt="API">
</a>
<a href="../../../../../resources/docs/reference/api-dark.svg#gh-dark-mode-only">
  <img src="../../../../../resources/docs/reference/api-dark.svg" alt="API">
</a>

<a id="type-method-names"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> method_names

**TYPE** · *protocol::http*

**Declaration synopsis:**

```cpp
struct method_names { ... };
```

Provides the http 1.1 method names specification.

**Source:** [include/protocol/http/common/method_names.h:36](../../../../../include/protocol/http/common/method_names.h#L36)

<a id="constant-kget"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kGet

**CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kGet[] = "GET";
```

Method  | Description GET     | Transfer a current representation of the target resource. HEAD    | Same as GET, but do not transfer the response content. POST    | Perform resource-specific processing on the request content. PUT     | Replace all current representations of the target resource | with the request content. DELETE  | Remove all

**Value:** `"GET"`

**Source:** [include/protocol/http/common/method_names.h:55](../../../../../include/protocol/http/common/method_names.h#L55)

<a id="constant-khead"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kHead

**CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kHead[] = "HEAD";
```

Defines the head constant.

**Value:** `"HEAD"`

**Source:** [include/protocol/http/common/method_names.h:56](../../../../../include/protocol/http/common/method_names.h#L56)

<a id="constant-kpost"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kPost

**CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kPost[] = "POST";
```

Defines the post constant.

**Value:** `"POST"`

**Source:** [include/protocol/http/common/method_names.h:57](../../../../../include/protocol/http/common/method_names.h#L57)

<a id="constant-kput"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kPut

**CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kPut[] = "PUT";
```

Defines the put constant.

**Value:** `"PUT"`

**Source:** [include/protocol/http/common/method_names.h:58](../../../../../include/protocol/http/common/method_names.h#L58)

<a id="constant-kdelete"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDelete

**CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kDelete[] = "DELETE";
```

Defines the delete constant.

**Value:** `"DELETE"`

**Source:** [include/protocol/http/common/method_names.h:59](../../../../../include/protocol/http/common/method_names.h#L59)

<a id="constant-kconnect"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kConnect

**CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kConnect[] = "CONNECT";
```

Defines the connect constant.

**Value:** `"CONNECT"`

**Source:** [include/protocol/http/common/method_names.h:60](../../../../../include/protocol/http/common/method_names.h#L60)

<a id="constant-koptions"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kOptions

**CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kOptions[] = "OPTIONS";
```

Defines the options constant.

**Value:** `"OPTIONS"`

**Source:** [include/protocol/http/common/method_names.h:61](../../../../../include/protocol/http/common/method_names.h#L61)

<a id="constant-ktrace"></a>

## <a href="../../../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kTrace

**CONSTANT** · *protocol::http::method_names*

```cpp
static constexpr char kTrace[] = "TRACE";
```

Defines the trace constant.

**Value:** `"TRACE"`

**Source:** [include/protocol/http/common/method_names.h:62](../../../../../include/protocol/http/common/method_names.h#L62)
