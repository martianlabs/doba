<a id="task-scheduler"></a>
<a href="../../../resources/docs/reference/common/task_scheduler/index.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/task_scheduler/index.svg" alt="task_scheduler">
</a>
<a href="../../../resources/docs/reference/common/task_scheduler/index-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/task_scheduler/index-dark.svg" alt="task_scheduler">
</a>

**Header:** [include/common/task_scheduler.h](../../../include/common/task_scheduler.h)

Provides the task scheduler type.

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common

[task_scheduler](#type-task-scheduler)

### <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> common::task_scheduler

[owner](#field-owner) ·
[post](#field-post) ·
[schedule](#field-schedule)

<a id="type-task-scheduler"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> task_scheduler

**TYPE** · *common*

**Declaration synopsis:**

```cpp
struct task_scheduler { ... };
```

Provides the task scheduler type.

**Source:** [include/common/task_scheduler.h:37](../../../include/common/task_scheduler.h#L37)

<a id="field-owner"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> owner

**FIELD** · *common::task_scheduler*

```cpp
void* owner{nullptr};
```

Stores the owner value.

**Value:** `{nullptr}`

**Source:** [include/common/task_scheduler.h:41](../../../include/common/task_scheduler.h#L41)

<a id="field-post"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> post

**FIELD** · *common::task_scheduler*

```cpp
void (*post)(void*, std::coroutine_handle<>){nullptr};
```

Stores the post value.

**Value:** `{nullptr}`

**Source:** [include/common/task_scheduler.h:42](../../../include/common/task_scheduler.h#L42)

<a id="field-schedule"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> schedule

**FIELD** · *common::task_scheduler*

```cpp
void (*schedule)(void*, std::chrono::steady_clock::time_point,
                   std::coroutine_handle<>){nullptr};
```

Stores the schedule value.

**Value:** `{nullptr}`

**Source:** [include/common/task_scheduler.h:43](../../../include/common/task_scheduler.h#L43)
