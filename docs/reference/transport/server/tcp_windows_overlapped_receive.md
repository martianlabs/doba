<a id="tcp-windows-overlapped-receive-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_receive/index-dark.svg">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_receive/index.svg" alt="Index for tcp_windows_overlapped_receive.h">
  </picture>
</h1>

**Header:** [include/transport/server/tcp_windows_overlapped_receive.h](../../../../include/transport/server/tcp_windows_overlapped_receive.h)

Provides the context transport component.

**transport::server**

- [context](#type-context) · *type*
- [overlapped_receive](#type-overlapped-receive) · *type*

**transport::server::overlapped_receive**

- [overlapped_receive](#constructor-overlapped-receive) · *constructor*
- [ctx](#field-ctx) · *field*

<a id="type-context"></a>

## context

🟠 **TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty, typename CNty>
struct context { ... };
```

Provides the context transport component.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **CNty:** `typename CNty`.

**Source:** [include/transport/server/tcp_windows_overlapped_receive.h:40](../../../../include/transport/server/tcp_windows_overlapped_receive.h#L40)

<a id="type-overlapped-receive"></a>

## overlapped_receive

🟠 **TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
template <protocol::contracts::engine ENty, typename CNty>
struct overlapped_receive : overlapped_base { ... };
```

Windows overlapped receive operation.

**Template parameters:**

- **ENty:** `protocol::contracts::engine ENty`.
- **CNty:** `typename CNty`.

**Source:** [include/transport/server/tcp_windows_overlapped_receive.h:50](../../../../include/transport/server/tcp_windows_overlapped_receive.h#L50)

<a id="constructor-overlapped-receive"></a>

## overlapped_receive

🟠 **CONSTRUCTOR** · *transport::server::overlapped_receive*

```cpp
overlapped_receive(std::shared_ptr<context<ENty, CNty>> context);
```

Constructs a overlapped_receive value.

**Parameters:**

- **context:** Context for this operation. Declaration: `std::shared_ptr<context<ENty, CNty>> context`.

**Source:** [include/transport/server/tcp_windows_overlapped_receive.h:54](../../../../include/transport/server/tcp_windows_overlapped_receive.h#L54)

<a id="field-ctx"></a>

## ctx

🟠 **FIELD** · *transport::server::overlapped_receive*

```cpp
std::shared_ptr<context<ENty, CNty>> ctx;
```

Stores the ctx value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/transport/server/tcp_windows_overlapped_receive.h:59](../../../../include/transport/server/tcp_windows_overlapped_receive.h#L59)
