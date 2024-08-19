#include "tt.h"
#include "../common/options.h"
#include "../common/stat.h"

namespace search {
#define TTENTRY_KEY16(key) static_cast<uint16_t>((key))
#define TTENTRY_KEY32(key) static_cast<uint32_t>((key))
#define TTDATA_BND_TYPE(dt) static_cast<uint8_t>((dt)) >> 14
#define TTDATA_GEN(dt) static_cast<int16_t>(dt & 0xFF)
#define TTDATA_HORIZON(dt) static_cast<uint16_t>(dt & 0x1F) >> 8
#define TTDATA_WRT(gen, hor, bnd) uint16_t((gen) & 0xFF) | (uint16_t((hor) & 0x1F) << 8) | (uint16_t((bnd) & 0x03) << 14)
#define TTDATA_SCORE(dt) static_cast<int16_t>(dt & 0x1FF)


TTable::TTable(const common::Options& opts, common::Stat& stat) noexcept 
    : m_size(opts.AvailMemTT * 1024/sizeof(TTChain)), m_stat(stat), m_age(0) {
    m_ttable = new TTChain[m_size];
}

TTable::~TTable() { delete[] m_ttable; }


// todo: key32 -> key16 is possible?
auto TTable::probe(uint64_t key) noexcept -> TTDescriptor {
    std::size_t idx = key % m_size;
    auto& chain = m_ttable[idx];
    auto key32 = TTENTRY_KEY32(key);

    TTEntry* ent = nullptr;
    uint8_t bound = 0x00;

    for(auto& entry : chain.entries) {
        if(entry.key == key32) {
            ent = &entry;
            bound = entry.bound;
        }
    }

    if(!ent) {
        ent = &m_ttable[idx].entries[0];
        for (unsigned i=1; i<std::size(m_ttable[idx].entries); i++) {
            if (ent->age < m_ttable[idx].entries[i].age)
                ent = &m_ttable[idx].entries[i];
        }
        ent->key = key32;
    }

    return TTDescriptor(ent, m_age, bound);
}



void TTable::incrementAge() noexcept {
    m_age++;
}


} // namespace search
