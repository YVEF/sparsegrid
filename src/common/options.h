#ifndef INCLUDE_COMMON_OPTIONS_H_
#define INCLUDE_COMMON_OPTIONS_H_

#include "../core/defs.h"
#define DEFAULT_CORES_NUMBER 1u
#define DEFAULT_MAX_DEPTH_PLY 25u
#define DEFAULT_AVAIL_MEM_KB 2048

namespace common {
struct Options {
    unsigned Cores = DEFAULT_CORES_NUMBER;
    unsigned MaxDepthPly = DEFAULT_MAX_DEPTH_PLY;
    unsigned AvailableMem = DEFAULT_AVAIL_MEM_KB;
    unsigned AvailMemTT = AvailableMem/2;
    unsigned AvailMemMTD = AvailableMem/2;
    PColor EngineSide = PColor::B;
};
} // namespace common

#endif  // INCLUDE_COMMON_OPTIONS_H_
