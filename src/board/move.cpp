#include "move.h"
#include "../dbg/sg_assert.h"
#include "board_state.h"


namespace brd {

void MoveList::push(Move moves) noexcept {
    SG_ASSERT(m_ptr < std::size(m_data));
    m_data[m_ptr++] = moves;
}


Move MoveList::pop() noexcept {
    SG_ASSERT(m_ptr);
    Move mv = m_data[--m_ptr];
    return mv;
}

Move mkMove(SQ from, SQ to) noexcept {
    return Move{from, to, 0x00, false, false};
}

Move mkCastling(SQ from, CastlingType ct) noexcept {
    return Move{from, 0x00, (uint8_t)ct, false, false};
}

Move mkEnpass(SQ from, SQ to) noexcept {
    return Move{from, to, 0x00, true, false};
}


brd::Move recognizeMove(SQ from, SQ to, const brd::Board& board) noexcept {
    if (!board.empty(to))
        return brd::mkMove(from, to);

    auto distance = dist(from, to);
    auto kind = board.getKind(1ull << from);
    // ========= castling test
    if (kind == PKind::pK && distance == 2) {
        return brd::mkCastling(from, (from < to ? brd::CastlingType::C_SHORT : brd::CastlingType::C_LONG));
    }

    // ========= enpassant test
    if (kind == PKind::pP && distance % 8)
        return brd::mkEnpass(from, to);

    return brd::mkMove(from, to);
}


} // namespace brd


