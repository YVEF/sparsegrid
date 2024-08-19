#ifndef INCLUDE_SEARCH_TT_H_
#define INCLUDE_SEARCH_TT_H_
#include <cstdint>
#include "../core/defs.h"
#include "../board/move.h"

namespace common { struct Options; struct Stat; }

namespace search {

static constexpr uint8_t UPPER_BND = 0x01;
static constexpr uint8_t LOWER_BND = 0x02;
static constexpr uint8_t EXACT_BND = 0x03;

struct TTEntry {
//    uint16_t key;
    uint32_t key;
    Score score;
    // |00|00 0000 |0000 0000
    // |  |        ----------- age (8)
    // |  -------------------- horizon (6)
    // ----------------------- bound type (2)
    uint16_t age : 8;
    uint16_t horizon : 6;
    uint16_t bound : 2;
    brd::Move hashMove;
};
// static_assert(sizeof(TTEntry) == 8, "TTEntry layout");



struct TTChain {
    TTEntry entries[4];
};
// static_assert(sizeof(TTChain) == 32, "Packer Move Size");

struct TTDescriptor {
    explicit TTDescriptor(TTEntry* hdl, uint8_t gen, uint8_t bound = 0) noexcept
        : m_bound(bound), m_handle(hdl), m_age(gen) {}

    bool hit() const noexcept { return static_cast<bool>(m_bound); }
    void write(Score score, int boundType, unsigned depth, const brd::Move& move) noexcept;
    const TTEntry* entry() const noexcept { return m_handle; }
    uint8_t bound() const noexcept { return m_bound; }

private:
    uint8_t     m_bound;
    TTEntry*    m_handle;
    uint8_t     m_age;
};

inline void TTDescriptor::write(Score score, int boundType, unsigned depth, const brd::Move& move) noexcept {
    m_handle->score = score;
    m_handle->age = m_age;
    m_handle->bound = boundType;
    m_handle->horizon = depth;
    m_handle->hashMove = move;
}

class TTable {
public:
    explicit TTable(const common::Options& opts, common::Stat& stat) noexcept;
    ~TTable();

    TTDescriptor probe(uint64_t key) noexcept;
    void incrementAge() noexcept;
private:
    TTChain*        m_ttable; // the handle chain
    std::size_t     m_size; // the size of tt
    common::Stat&   m_stat;
    uint8_t         m_age; // generation

};

} // namespace search



#endif  // INCLUDE_SEARCH_TT_H_

