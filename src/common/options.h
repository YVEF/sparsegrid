#ifndef INCLUDE_COMMON_OPTIONS_H_
#define INCLUDE_COMMON_OPTIONS_H_

#include "../core/defs.h"
#include <string>

#define DEFAULT_CORES_NUMBER 1u
#define DEFAULT_MAX_DEPTH_PLY 25u
#define DEFAULT_TT_MEM_KB (4*1024)

namespace common {
struct Options {
    unsigned Cores = DEFAULT_CORES_NUMBER;
    unsigned MaxDepthPly = DEFAULT_MAX_DEPTH_PLY;
    unsigned AvailMemTT = DEFAULT_TT_MEM_KB;
    PColor EngineSide = PColor::B;
    std::string NNStateFile;
};

std::string getNNGzipFile();
} // namespace common

#endif  // INCLUDE_COMMON_OPTIONS_H_
