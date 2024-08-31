#ifndef INCLUDE_CORE_CALLER_THREAD_EXECUTOR_H_
#define INCLUDE_CORE_CALLER_THREAD_EXECUTOR_H_
#include <future>
#include <functional>
#include "ThreadContext.h"

namespace common { struct Options; }
namespace exec {
class CallerThreadExecutor {
public:
    explicit CallerThreadExecutor(const common::Options& opts) noexcept;
    decltype(auto) send(auto&& task) noexcept;
};


decltype(auto) CallerThreadExecutor::send(auto&& task) noexcept {
#define EXEC_FWD_TASK(t) std::forward<decltype((t))>((t))

    auto tc = ThreadContext{};
    using return_type = decltype(std::invoke(EXEC_FWD_TASK(task), tc));
    std::promise<return_type> prom{};
    prom.set_value(std::invoke(EXEC_FWD_TASK(task), tc));
    return prom.get_future();
}

CallerThreadExecutor::CallerThreadExecutor(const common::Options& opts) noexcept {

}


} // namespace exec
#endif  // INCLUDE_CORE_CALLER_THREAD_EXECUTOR_H_
