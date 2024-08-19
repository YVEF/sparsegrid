#include "tm.h"

namespace search {
void TimeManager::setTimeout(uint64_t limit) noexcept {
    m_timelimMs = limit;
}

bool TimeManager::timeout() const noexcept {
    if(!m_terminated) {
        auto now = clock_t::now();
        m_terminated = m_timelimMs < (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start).count();
    }
    return m_terminated;
}

void TimeManager::stop() noexcept {
    m_terminated = true;
}

void TimeManager::startCounting() noexcept {
    m_terminated = false;
    m_start = clock_t::now();
}
} // namespace search
