#ifndef SPARSEGRID_THREADPOOLEXECUTOR_H
#define SPARSEGRID_THREADPOOLEXECUTOR_H
#include <mutex>
#include <memory>
#include <queue>
#include <functional>
#include <thread>
#include <condition_variable>
#include <future>
#include "ThreadContext.h"
#include <unordered_map>
#include <iostream>
#include "../dbg/sg_assert.h"
#include <optional>


namespace common { struct Options; }
namespace exec {

class ThreadPoolExecutor {
public:
    explicit ThreadPoolExecutor(const common::Options& opts) noexcept;
    ~ThreadPoolExecutor();

    /*
     * Send the task to workers pool
     */
    template<typename T> auto send(T&& task) noexcept;

    /*
     * Send the task only if there are idle workers available to handle on
     */
    template<typename T> auto try_send(T&& task) noexcept -> std::optional<decltype(send(std::forward<T>(task)))>;

    /*
     * Total number of workers
     */
    std::size_t capacity() noexcept;

private:
    std::mutex m_mtx;
    std::queue<std::function<void()>> m_jobs;
    std::vector<std::jthread> m_workers;
    std::condition_variable m_cv;
    std::atomic_bool m_active{true};
    std::atomic_uint m_nonCompletedJobs{0};

    void worker_loop_() noexcept;
    template<typename T> bool send_(T&& task, auto& future, bool restricted = false) noexcept;
};


template<typename T>
bool ThreadPoolExecutor::send_(T&& task, auto& future, bool restricted) noexcept {
    SG_ASSERT(!m_workers.empty());

    // race condition actually happens here
    if (restricted && m_nonCompletedJobs.load(std::memory_order_acquire) >= m_workers.size())
        return false;

    using return_type = decltype(std::invoke(std::forward<T>(task)));
    auto prom = std::make_shared<std::promise<return_type>>();
    future = prom->get_future();
    std::function<void()> fnc(
        [prom = std::move(prom),
            task = std::forward<T>(task),
            &ncj = m_nonCompletedJobs]() mutable {
            prom->set_value(std::invoke(std::forward<T>(task)));
            ncj.fetch_sub(1);
        });

    {
        std::lock_guard lock(m_mtx);
        if (restricted && m_nonCompletedJobs.load(std::memory_order_relaxed) >= m_workers.size())
            return false;

        m_jobs.emplace(std::move(fnc));
        m_nonCompletedJobs.fetch_add(1);
    }
    m_cv.notify_one();
    return true;
}


template<typename T>
auto ThreadPoolExecutor::send(T&& task) noexcept {
    using return_type = decltype(std::invoke(std::forward<T>(task)));
    std::future<return_type> future;
    send_(std::forward<T>(task), future, false);
    return future;
}

template<typename T>
auto ThreadPoolExecutor::try_send(T&& task) noexcept -> std::optional<decltype(send(std::forward<T>(task)))> {
    using return_type = decltype(std::invoke(std::forward<T>(task)));
    std::future<return_type> future{};
    if (send_(std::forward<T>(task), future, true)) {
        return std::optional{std::move(future)};
    }
//    std::cout << "no way" << std::endl;
    return std::nullopt;
    //return std::optional<decltype(future)>{};
}

} // exec

#endif //SPARSEGRID_THREADPOOLEXECUTOR_H
