#ifndef INCLUDE_COMMON_STAT_H_
#define INCLUDE_COMMON_STAT_H_

#include <cstdint>
namespace common {
struct Stat {
    uint64_t TTMatch = 0;
    uint64_t NodesSearched = 0;

    void resetSingleSearch() noexcept;
};
} // namespace common

#endif  // INCLUDE_COMMON_STAT_H_
