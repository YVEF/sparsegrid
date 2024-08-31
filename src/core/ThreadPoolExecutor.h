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


namespace common { struct Options; }
namespace exec {

class ThreadPoolExecutor {
public:
    explicit ThreadPoolExecutor(const common::Options& opts) noexcept;
    ~ThreadPoolExecutor();
    decltype(auto) send(auto&& task) noexcept;

private:
//    const common::Options& m_opts;
    std::mutex m_mtx;
    std::queue<std::packaged_task<void(ThreadContext&)>> m_jobs;
    std::vector<std::jthread> m_workers;
    std::vector<ThreadContext> m_contexts;
//    std::unordered_map<unsigned, unsigned> tidIdMap;
    std::condition_variable m_cv;
    std::atomic_bool m_active;

    void worker_loop_(ThreadContext& ctx) noexcept;
};

decltype(auto) ThreadPoolExecutor::send(auto&& task) noexcept {
    using return_type = decltype(std::invoke(task, std::declval<ThreadContext&>()));

    auto ret_prom = std::make_shared<std::promise<return_type>>();
    auto future = ret_prom.get_future();

    std::packaged_task<void()> ptask(
        [prom = std::move(ret_prom),
         task = std::forward<decltype(task)>(task)](auto& ctx) {
        auto res = std::invoke(std::forward<decltype(task)>(task), ctx);
        prom->set_value(res);
    });

    {
        std::lock_guard lock(m_mtx);
        m_jobs.emplace(std::move(ptask));
    }
    m_cv.notify_one();
    return future;
}

} // exec

#endif //SPARSEGRID_THREADPOOLEXECUTOR_H
