#ifndef INCLUDE_CORE_CALLER_THREAD_EXECUTOR_H_
#define INCLUDE_CORE_CALLER_THREAD_EXECUTOR_H_
#include <future>
#include <functional>
#include "ThreadContext.h"

namespace common { struct Options; }
namespace exec {
class CallerThreadExecutor {
public:
    explicit CallerThreadExecutor(const common::Options&) noexcept;
    template<typename T>
    auto send(T&& task) noexcept;
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


} // namespace exec
#endif  // INCLUDE_CORE_CALLER_THREAD_EXECUTOR_H_
