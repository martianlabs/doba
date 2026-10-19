<a id="task-scheduler-h"></a>
<h1>
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="../../../resources/docs/reference/common/task_scheduler/index-dark.svg">
    <img src="../../../resources/docs/reference/common/task_scheduler/index.svg" alt="Index for task_scheduler.h">
  </picture>
</h1>

**Header:** [include/common/task_scheduler.h](../../../include/common/task_scheduler.h)

Provides the task scheduler type.

**common**

- [task_scheduler](#type-task-scheduler) · *type*

**common::task_scheduler**

- [owner](#field-owner) · *field*
- [post](#field-post) · *field*
- [schedule](#field-schedule) · *field*

<a id="type-task-scheduler"></a>

## task_scheduler

🟠 **TYPE** · *common*

**Declaration synopsis:**

```cpp
struct task_scheduler { ... };
```

Provides the task scheduler type.

**Source:** [include/common/task_scheduler.h:37](../../../include/common/task_scheduler.h#L37)

<a id="field-owner"></a>

## owner

🟠 **FIELD** · *common::task_scheduler*

```cpp
void* owner{nullptr};
```

Stores the owner value.

**Value:** `{nullptr}`

**Source:** [include/common/task_scheduler.h:41](../../../include/common/task_scheduler.h#L41)

<a id="field-post"></a>

## post

🟠 **FIELD** · *common::task_scheduler*

```cpp
void (*post)(void*, std::coroutine_handle<>){nullptr};
```

Stores the post value.

**Value:** `{nullptr}`

**Source:** [include/common/task_scheduler.h:42](../../../include/common/task_scheduler.h#L42)

<a id="field-schedule"></a>

## schedule

🟠 **FIELD** · *common::task_scheduler*

```cpp
void (*schedule)(void*, std::chrono::steady_clock::time_point,
                   std::coroutine_handle<>){nullptr};
```

Stores the schedule value.

**Value:** `{nullptr}`

**Source:** [include/common/task_scheduler.h:43](../../../include/common/task_scheduler.h#L43)
