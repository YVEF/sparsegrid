#include "polyglot.h"
#include "../board/board_state.h"
#include <array>
#include "../common/options.h"
#include "../dbg/sg_assert.h"


namespace adapters::polyglot {
extern const std::array<uint64_t, 781>& poly_keys;

static int convertPiece(PKind kind, PColor color) noexcept {
    int res = 0;
    switch(kind) {
        case PKind::pP: res = 0; break;
        case PKind::pN: res = 2; break;
        case PKind::pB: res = 4; break;
        case PKind::pR: res = 6; break;
        case PKind::pQ: res = 8; break;
        case PKind::pK: res = 10; break;
        default: SG_ASSERT(false); break;
    }
    return res + (int)color;
}

static uint64_t bldTurnHash(const brd::BoardState& state) noexcept {
    if (getNextPlayerColor(state)) return poly_keys[780];
    return 0;
}

static uint64_t getPieceKey(PKind kind, PColor color, SQ sq) noexcept {
    return poly_keys[64 * convertPiece(kind, color) + sq];
}

static uint64_t bldEnpassantHash(const brd::BoardState& state) {
    uint64_t hash   = 0;
    constexpr int offset = 772;

    SQ pos = 0x00;
    if (!state.buildFromFen())
        pos = state.PG_enpassPos();
    else {
        auto sq = state.FenGetEnpass();
        if (sq) {
            SG_ASSERT(state.FenGetNextPlayer().has_value());

            auto&& board = state.getBoard();
            int coeff = state.FenGetNextPlayer().value() ? -8 : 8;
            const SQ pawnPos = sq + coeff;
            const PColor pawnColor = board.getColor(1ull << pawnPos);
            const BB leftPawnMask = 1ull << (pawnPos-1);
            const BB rightPawnMask = 1ull << (pawnPos + 1);

            if (!(leftPawnMask & NFile::fA)) {
                auto lKind = board.getKind(leftPawnMask);
                if (lKind != PKind::None && pawnColor != board.getColor(leftPawnMask))
                    pos = sq;
            }
            if (!pos && !(rightPawnMask & NFile::fH)) {
                auto rKind = board.getKind(rightPawnMask);
                if (rKind != PKind::None && pawnColor != board.getColor(rightPawnMask))
                    pos = sq;
            }
        }
    }

    if(!pos) return hash;

    uint8_t file = pos % 8;
    hash ^= poly_keys[offset + file];
    return hash;
}


static uint64_t bldCastleHash(const brd::BoardState& state) {
    constexpr int offset = 768;
    uint64_t      hash   = 0;

    unsigned mask = state.buildFromFen() ? state.getFenCastlingMask() : state.PG_possibleCastlMask();
    if(mask & FEN_SHORT_WHITE_CASTLE_MASK) hash ^= poly_keys[offset];
    if(mask & FEN_LONG_WHITE_CASTLE_MASK) hash ^= poly_keys[offset+1];
    if(mask & FEN_SHORT_BLACK_CASTLE_MASK) hash ^= poly_keys[offset+2];
    if(mask & FEN_LONG_BLACK_CASTLE_MASK) hash ^= poly_keys[offset+3];

    return hash;
}


static uint64_t bldPieceHash(const brd::BoardState& state) noexcept {
    uint64_t hash = 0;
    auto&& board = state.getBoard();
    for (SQ sq = 0; sq < BRD_SIZE; sq++) {
        if (board.empty(sq)) continue;
        BB mask = 1ull << sq;
        PColor color = board.getColor(mask);
        PKind kind = board.getKind(mask);
        hash ^= getPieceKey(kind, color, sq);
    }

    return hash;
}

uint64_t makeKey(const brd::BoardState& state) noexcept {
    return bldPieceHash(state)
           ^ bldCastleHash(state)
           ^ bldTurnHash(state)
           ^ bldEnpassantHash(state);
}

} // namespace adapters::polyglot
