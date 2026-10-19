<a id="context-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../../resources/docs/reference/protocol/http/v11/context/index-dark.svg">
    <img src="../../../../../resources/docs/reference/protocol/http/v11/context/index.svg" alt="Index for context.h">
  </picture>
</h1>

**Header:** [include/protocol/http/v11/context.h](../../../../../include/protocol/http/v11/context.h)

The aggregated view the transversal rules operate on. Where an intra- header interpreter sees a single header in isolation, a rule needs to reason across several: whether a header was present at all, how two headers combine, and the connection/policy state the interpreters already derived. deserialize() fills this context during its single header

**protocol::http::v11**

- [context](#type-context) · *type*

**protocol::http::v11::context**

- [policies](#type-policies) · *type*
- [connection](#field-connection) · *field*
- [has_content_length](#field-has-content-length) · *field*
- [multiple_content_length](#field-multiple-content-length) · *field*
- [content_length](#field-content-length) · *field*
- [has_transfer_encoding](#field-has-transfer-encoding) · *field*
- [has_host](#field-has-host) · *field*
- [multiple_host](#field-multiple-host) · *field*
- [host](#field-host) · *field*
- [has_target_authority](#field-has-target-authority) · *field*
- [target_authority](#field-target-authority) · *field*
- [forwarding_hops](#field-forwarding-hops) · *field*
- [rejection_reason](#field-rejection-reason) · *field*

<a id="type-context"></a>

## context

🟠 **TYPE** · *protocol::http::v11*

**Declaration synopsis:**

```cpp
struct context { ... };
```

The aggregated view the transversal rules operate on. Where an intra- header interpreter sees a single header in isolation, a rule needs to reason across several: whether a header was present at all, how two headers combine, and the connection/policy state the interpreters already derived. deserialize() fills this context during its single header

**Source:** [include/protocol/http/v11/context.h:58](../../../../../include/protocol/http/v11/context.h#L58)

<a id="type-policies"></a>

## policies

🟠 **TYPE** · *protocol::http::v11::context*

**Declaration synopsis:**

```cpp
struct policies { ... };
```

Policies coming from the inbound configuration; they are not derived from the request.

**Source:** [include/protocol/http/v11/context.h:61](../../../../../include/protocol/http/v11/context.h#L61)

<a id="field-connection"></a>

## connection

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
v11::connection connection;
```

The mutable hop-by-hop connection state derived from the request.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/context.h:63](../../../../../include/protocol/http/v11/context.h#L63)

<a id="field-has-content-length"></a>

## has_content_length

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
bool has_content_length = false;
```

Content-Length and Transfer-Encoding are mutually exclusive; the rules check for their presence and the number of Content-Length headers. The parsed Content-Length value is stored for the rules to reason about it.

**Value:** `false`

**Source:** [include/protocol/http/v11/context.h:67](../../../../../include/protocol/http/v11/context.h#L67)

<a id="field-multiple-content-length"></a>

## multiple_content_length

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
bool multiple_content_length = false;
```

Stores the multiple content length value.

**Value:** `false`

**Source:** [include/protocol/http/v11/context.h:68](../../../../../include/protocol/http/v11/context.h#L68)

<a id="field-content-length"></a>

## content_length

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
std::size_t content_length = 0;
```

Stores the content length value.

**Value:** `0`

**Source:** [include/protocol/http/v11/context.h:69](../../../../../include/protocol/http/v11/context.h#L69)

<a id="field-has-transfer-encoding"></a>

## has_transfer_encoding

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
bool has_transfer_encoding = false;
```

Stores the has transfer encoding value.

**Value:** `false`

**Source:** [include/protocol/http/v11/context.h:70](../../../../../include/protocol/http/v11/context.h#L70)

<a id="field-has-host"></a>

## has_host

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
bool has_host = false;
```

Host and Target-Authority are mutually exclusive; the rules check for their presence and the number of Host headers. The parsed values are stored for the rules to reason about them.

**Value:** `false`

**Source:** [include/protocol/http/v11/context.h:74](../../../../../include/protocol/http/v11/context.h#L74)

<a id="field-multiple-host"></a>

## multiple_host

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
bool multiple_host = false;
```

Stores the multiple host value.

**Value:** `false`

**Source:** [include/protocol/http/v11/context.h:75](../../../../../include/protocol/http/v11/context.h#L75)

<a id="field-host"></a>

## host

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
parsed_host_port host;
```

Stores the host value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/context.h:76](../../../../../include/protocol/http/v11/context.h#L76)

<a id="field-has-target-authority"></a>

## has_target_authority

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
bool has_target_authority = false;
```

Stores the has target authority value.

**Value:** `false`

**Source:** [include/protocol/http/v11/context.h:77](../../../../../include/protocol/http/v11/context.h#L77)

<a id="field-target-authority"></a>

## target_authority

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
parsed_host_port target_authority;
```

Stores the target authority value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/protocol/http/v11/context.h:78](../../../../../include/protocol/http/v11/context.h#L78)

<a id="field-forwarding-hops"></a>

## forwarding_hops

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
std::size_t forwarding_hops = 0;
```

The number of forwarding hops across Via / Forwarded / X-Forwarded-For.

**Value:** `0`

**Source:** [include/protocol/http/v11/context.h:80](../../../../../include/protocol/http/v11/context.h#L80)

<a id="field-rejection-reason"></a>

## rejection_reason

🟠 **FIELD** · *protocol::http::v11::context*

```cpp
martianlabs::doba::protocol::http::v11::rejection_reason rejection_reason =
      martianlabs::doba::protocol::http::v11::rejection_reason::kNone;
```

Why the last verdict::kReject happened, at the granularity the HTTP/1.1 layer needs to pick a status code. Set by the interpreter/checker that rejects the request; left at kNone for syntactic errors, which default to 400 Bad Request.

**Value:** `martianlabs::doba::protocol::http::v11::rejection_reason::kNone`

**Source:** [include/protocol/http/v11/context.h:85](../../../../../include/protocol/http/v11/context.h#L85)
