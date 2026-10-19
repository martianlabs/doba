<a id="tcp-windows-overlapped-base-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_base/index-dark.svg">
    <img src="../../../../resources/docs/reference/transport/server/tcp_windows_overlapped_base/index.svg" alt="Index for tcp_windows_overlapped_base.h">
  </picture>
</h1>

**Header:** [include/transport/server/tcp_windows_overlapped_base.h](../../../../include/transport/server/tcp_windows_overlapped_base.h)

Base state for Windows overlapped I/O.

**transport::server**

- [io_type](#enum-io-type) · *enum*
- [overlapped_base](#type-overlapped-base) · *type*

**transport::server::overlapped_base**

- [overlapped_base](#constructor-overlapped-base) · *constructor*
- [get_type](#method-get-type) · *method*

<a id="enum-io-type"></a>

## io_type

🟠 **ENUM** · *transport::server*

```cpp
enum class io_type : uint8_t { kAccept, kSend, kReceive, kOutput };
```

Enumerates io type values.

**Values:**

- **kAccept:** Accept. Declaration: `kAccept`.
- **kSend:** Send. Declaration: `kSend`.
- **kReceive:** Receive. Declaration: `kReceive`.
- **kOutput:** Output. Declaration: `kOutput`.

**Source:** [include/transport/server/tcp_windows_overlapped_base.h:38](../../../../include/transport/server/tcp_windows_overlapped_base.h#L38)

<a id="type-overlapped-base"></a>

## overlapped_base

🟠 **TYPE** · *transport::server*

**Declaration synopsis:**

```cpp
struct overlapped_base : OVERLAPPED { ... };
```

Base state for Windows overlapped I/O.

**Source:** [include/transport/server/tcp_windows_overlapped_base.h:47](../../../../include/transport/server/tcp_windows_overlapped_base.h#L47)

<a id="constructor-overlapped-base"></a>

## overlapped_base

🟠 **CONSTRUCTOR** · *transport::server::overlapped_base*

```cpp
overlapped_base(io_type in_type);
```

Constructs a overlapped_base value.

**Parameters:**

- **in_type:** Initial value supplied to the constructor. Declaration: `io_type in_type`.

**Source:** [include/transport/server/tcp_windows_overlapped_base.h:51](../../../../include/transport/server/tcp_windows_overlapped_base.h#L51)

<a id="method-get-type"></a>

## get_type

🟠 **METHOD** · *transport::server::overlapped_base*

```cpp
io_type get_type() const;
```

Returns type.

**Returns:** A value of type `io_type`.

**Source:** [include/transport/server/tcp_windows_overlapped_base.h:55](../../../../include/transport/server/tcp_windows_overlapped_base.h#L55)
