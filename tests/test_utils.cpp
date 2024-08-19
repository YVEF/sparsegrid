#include "test_utils.h"
#include <algorithm>
#include <unordered_set>



void preserveOnlyPositions(brd::Board& brd, std::initializer_list<uint8_t> positions) noexcept {
    std::unordered_set<uint8_t> pos = positions;
    for(uint8_t sq =0; sq<64; sq++) {
        if(pos.count(sq) || brd.empty(sq)) continue;
        brd.kill(sq);
    }
}
