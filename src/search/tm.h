#ifndef INCLUDE_SEARCH_TM_H_
#define INCLUDE_SEARCH_TM_H_

#include <cstdint>
#include <chrono>
namespace search {
class TimeManager {
public:
    bool timeout() const noexcept;
    void setTimeout(uint64_t) noexcept;
    void startCounting() noexcept;
    void stop() noexcept;
private:
    using clock_t = std::chrono::steady_clock;
    uint64_t            m_timelimMs;
    clock_t::time_point m_start;
    mutable bool        m_terminated;
};

} // namespace search

#endif  // INCLUDE_SEARCH_TM_H_
