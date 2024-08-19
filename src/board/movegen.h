#ifndef INCLUDE_BOARD_MOVEGEN_H_
#define INCLUDE_BOARD_MOVEGEN_H_
#include <cstdint>
#include "../core/defs.h"

namespace brd { class Board; class BoardState; }

namespace movegen {
void init();
uint64_t getRookOccupancy(SQ sq, BB occupied) noexcept;
uint64_t getBishopOccupancy(SQ sq, BB occupied) noexcept;

uint64_t getKingOccupancy(SQ sq) noexcept;

uint64_t getKnightOccupancy(SQ sq) noexcept;

template<PColor Color>
uint64_t getPawnAttacks(SQ sq) noexcept;

template<PColor Color>
uint64_t getPawnMoves(SQ sq, const brd::BoardState&) noexcept;

template<PColor Color, PKind Kind>
BB getPieceOccupancy(SQ sq, BB occupied) noexcept;

/**
 * Returns the "to" square if enpassant possible
 */
template<PColor Color>
uint8_t getEnpassantAttack(SQ sq, const brd::BoardState& state) noexcept;

template <PColor Color, PKind Kind>
BB getPieceOccupancy(SQ sq, BB occupied) noexcept {
    if constexpr (Kind == PKind::pR) {
        return getRookOccupancy(sq, occupied);
    }
    if constexpr (Kind == PKind::pB) {
        return getBishopOccupancy(sq, occupied);
    }
    if constexpr (Kind == PKind::pQ) {
        return movegen::getRookOccupancy(sq, occupied) 
            | movegen::getBishopOccupancy(sq, occupied);
    }
    if constexpr (Kind == PKind::pK) {
        return movegen::getKingOccupancy(sq);
    }
    if constexpr (Kind == PKind::pN) {
        return movegen::getKnightOccupancy(sq);
    }
}

} // namespace movegen

#endif  // INCLUDE_BOARD_MOVEGEN_H_
