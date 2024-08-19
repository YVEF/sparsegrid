#ifndef INCLUDE_CORE_CALLER_THREAD_EXECUTOR_H_
#define INCLUDE_CORE_CALLER_THREAD_EXECUTOR_H_
#include <future>
#include <optional>
#include <functional>
#include "ThreadContext.h"

namespace common { struct Options; }
namespace exec {
class CallerThreadExecutor {
public:
    explicit CallerThreadExecutor(const common::Options&) noexcept;
    template<typename T> auto send(T&& task) noexcept;
    template<typename T> auto try_send(T&& task) noexcept;
    std::size_t capacity() noexcept;
};


template<typename T>
auto CallerThreadExecutor::send(T&& task) noexcept {
#define EXEC_FWD_TASK(t) std::forward<T>((t))

    auto tc = ThreadContext{};
    using return_type = decltype(std::invoke(EXEC_FWD_TASK(task), tc));
    std::promise<return_type> prom{};
    prom.set_value(std::invoke(EXEC_FWD_TASK(task), tc));
    return prom.get_future();
}

template<typename T>
auto CallerThreadExecutor::try_send(T&& task) noexcept {
    auto tc = ThreadContext{};
    using return_type = decltype(std::invoke(std::forward<T>(task), tc));
    std::optional<std::future<return_type>> result{send(std::forward<T>(task))};
    return result;
}

} // namespace exec
#endif  // INCLUDE_CORE_CALLER_THREAD_EXECUTOR_H_
