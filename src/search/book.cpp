#include "book.h"
#include <fstream>
#include "../core/defs.h"
#include "../board/board_state.h"
#include "../adapters/polyglot.h"
#include "../common/options.h"

namespace search {

// todo: decouple book.h and adapters

struct poly_entry_ {
    uint64_t key;
    uint16_t move;
    uint16_t weight;
    uint32_t learn;
};

Book::Book(const common::Options& opts) noexcept : m_opts(opts) {}


void Book::read(std::string_view path) {
    std::ifstream filestr(path.data(), std::ios::binary | std::ios::ate);

    size_t size  = filestr.tellg();
    size_t count = size / sizeof(poly_entry_);
    m_entries.reserve(count);
    filestr.seekg(0);
    for (size_t i = 0; i < count; i++) {
        poly_entry_ entry;
        SgPolyEntry sg_entry;

        filestr.read((char*) &entry, sizeof(poly_entry_));
        sg_entry.key        = __builtin_bswap64(entry.key);
        sg_entry.weight     = __builtin_bswap16(entry.weight);
        auto pmove          = __builtin_bswap16(entry.move);
        // entry.learn      = __builtin_bswap32(entry.learn);

        auto tf = static_cast<uint8_t>(pmove & 0x07);
        auto tr = static_cast<uint8_t>((pmove >> 3) & 0x07);
        auto ff = static_cast<uint8_t>((pmove >> 6) & 0x07);
        auto fr = static_cast<uint8_t>((pmove >> 9) & 0x07);
        // auto pp = static_cast<uint8_t>((pmove >> 12) & 0x07); // ???

        sg_entry.move_from = ff + fr*8;
        sg_entry.move_to = tf + tr*8;
        if(sg_entry.move_from == sg_entry.move_to)
            continue;

        m_entries.push_back(sg_entry);
    }
    filestr.close();
}


bool Book::probe(const brd::BoardState& state, brd::Move& move) const noexcept {
    auto key = adapters::polyglot::makeKey(state, m_opts);
    // todo: use table

    SgPolyEntry result{};
    for(auto& ent : m_entries) {
        if(ent.key == key && ent.weight > result.weight)
            result = ent;
    }

    if(!result.move_to || !result.move_from)
        return false;


    move = recognizeMove(result.move_from, result.move_to, state.getBoard());
    return true;
}


} // namespace search