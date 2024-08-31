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


namespace common { struct Options; }
namespace exec {

class ThreadPoolExecutor {
public:
    explicit ThreadPoolExecutor(const common::Options& opts) noexcept;
    ~ThreadPoolExecutor();
    template<typename T>
    auto send(T&& task) noexcept;

private:
//    const common::Options& m_opts;
    std::mutex m_mtx;
//    std::queue<std::packaged_task<void(ThreadContext&)>> m_jobs;
    std::queue<std::function<void(ThreadContext&)>> m_jobs;
    std::vector<std::jthread> m_workers;
    std::vector<ThreadContext> m_contexts;
//    std::unordered_map<unsigned, unsigned> tidIdMap;
    std::condition_variable m_cv;
    std::atomic_bool m_active{true};

    void worker_loop_(ThreadContext& ctx) noexcept;
};

template<typename T>
auto ThreadPoolExecutor::send(T&& task) noexcept {
    SG_ASSERT(!m_workers.empty());

    using return_type = decltype(std::invoke(std::forward<T>(task), std::declval<ThreadContext&>()));
    auto prom = std::make_shared<std::promise<return_type>>();
    auto future = prom->get_future();
    std::function<void(ThreadContext&)> fnc(
        [prom = std::move(prom),
            task = std::forward<T>(task)](auto& tCtx) mutable {
            prom->set_value(std::invoke(std::forward<T>(task), tCtx));
        });

    {
        std::lock_guard lock(m_mtx);
        m_jobs.emplace(std::move(fnc));
    }
    m_cv.notify_one();
    return future;
}

} // exec

#endif //SPARSEGRID_THREADPOOLEXECUTOR_H
