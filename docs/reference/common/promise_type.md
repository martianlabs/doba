<a id="promise-type"></a>
<a href="../../../resources/docs/reference/common/promise_type/title.svg#gh-light-mode-only">
  <img src="../../../resources/docs/reference/common/promise_type/title.svg" alt="promise_type">
</a>
<a href="../../../resources/docs/reference/common/promise_type/title-dark.svg#gh-dark-mode-only">
  <img src="../../../resources/docs/reference/common/promise_type/title-dark.svg" alt="promise_type">
</a>

**Header:** [include/common/task_promise.h](../../../include/common/task_promise.h)

The promise used by task<void>, exposed through its promise_type alias.

<a id="type-promise-type"></a>

## <a href="../../../resources/docs/reference/entry-accent.svg#gh-light-mode-only"><img src="../../../resources/docs/reference/entry-accent.svg" alt="" width="4" height="20"></a><a href="../../../resources/docs/reference/entry-accent-dark.svg#gh-dark-mode-only"><img src="../../../resources/docs/reference/entry-accent-dark.svg" alt="" width="4" height="20"></a> task_promise

```cpp
struct task_promise;
```

| Member | Behavior |
| --- | --- |
| get_return_object() | Creates the task that owns this frame until adoption. |
| initial_suspend() | Returns std::suspend_always so construction does not run the body. |
| final_suspend() | Returns std::suspend_always so completion keeps the frame alive. |
| return_void() | Finishes the coroutine body. |
| unhandled_exception() | Keeps the exception for task_execution::error(). |
| await_transform(awaiter) | Accepts only yield_awaiter and sleep_awaiter. |
| error | The captured std::exception_ptr. |
| scheduler | The readiness/deadline hooks installed by task_execution. |

An unsupported awaiter throws std::logic_error before any of its hooks run.
The execution owner can inspect that error and release the frame.

The default scheduler hooks are null. `task_execution` installs them before
the first resume; manually resuming an unadopted task into a managed awaiter
is outside this contract.
