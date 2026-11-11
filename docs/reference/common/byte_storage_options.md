# byte_storage_options

**Header:** [include/common/byte_storage_options.h](../../../include/common/byte_storage_options.h)

Options shared by [byte_storage](byte_storage.md) and HTTP body writers.

The default memory threshold is 128 KiB. A write that exceeds it moves the
stored bytes to a temporary file; a zero threshold disables automatic spilling.
An empty `spill_dir` uses the system temporary directory. These options do not
set HTTP acceptance limits or reserve memory up front.

<a id="type-byte-storage-options"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> byte_storage_options

**TYPE** · *common*

**Declaration synopsis:**

```cpp
struct byte_storage_options { ... };
```

Controls when byte storage spills to a temporary file.

**Source:** [include/common/byte_storage_options.h](../../../include/common/byte_storage_options.h)

<a id="constant-kdefaultspilltodiskthreshold"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> kDefaultSpillToDiskThreshold

**CONSTANT** · *common::byte_storage_options*

```cpp
static constexpr std::size_t kDefaultSpillToDiskThreshold = 128 * 1024;
```

Defines the default spill to disk threshold constant.

**Value:** `128 * 1024`

**Source:** [include/common/byte_storage_options.h](../../../include/common/byte_storage_options.h)

<a id="field-spill-to-disk-threshold"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> spill_to_disk_threshold

**FIELD** · *common::byte_storage_options*

```cpp
std::size_t spill_to_disk_threshold = kDefaultSpillToDiskThreshold;
```

Stores the spill to disk threshold value.

**Value:** `kDefaultSpillToDiskThreshold`

**Source:** [include/common/byte_storage_options.h](../../../include/common/byte_storage_options.h)

<a id="field-spill-dir"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> spill_dir

**FIELD** · *common::byte_storage_options*

```cpp
std::string spill_dir;
```

Stores the spill dir value.

**Value:** No explicit initializer appears in this declaration.

**Source:** [include/common/byte_storage_options.h](../../../include/common/byte_storage_options.h)

