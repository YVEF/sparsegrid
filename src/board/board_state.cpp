#include "board_state.h"
#include "../dbg/sg_assert.h"
#include "../core/scores.h"
#include "../dbg/debugger.h"

#define CASTL_NEW_KING_POS(kingPos, castlType) ((castlType) == brd::CastlingType::C_SHORT ? (kingPos)+2 : (kingPos)-2)
#define CASTL_NEW_ROOK_POS(kingPos, castlType) (castlType == brd::CastlingType::C_SHORT ? (kingPos)+1 : (kingPos)-1)
#define CASTL_ORIG_ROOK_POS(kingPos, castlType) ((castlType) == brd::CastlingType::C_SHORT ? (kingPos)+3 : (kingPos)-4)

Score PieceScores[] = {PAWN_SCORE, DUMMY_SCORE, QUEEN_SCORE, BISHOP_SCORE, KNIGHT_SCORE};

namespace brd {
namespace details {
template<PColor Color, PKind Kind>
struct MG {
public:
    void run(Board& board, MoveList& mvList, const BoardState& state) {
        board.movegen<Color, Kind>(mvList, state);
        MG<Color, (PKind)(Kind+1)>{}.run(board, mvList, state);
    }
};

template<PColor Color>
struct MG<Color, PKind::pR> {
    void run(Board& board, MoveList& mvList, const BoardState& state) {
        board.movegen<Color, PKind::pR>(mvList, state);
        if constexpr (Color == PColor::B) {
            return;
        }
        MG<PColor::B, PKind::pP>{}.run(board, mvList, state);
    }
};
}

BoardState::BoardState(brd::Board&& board) noexcept 
: m_board(std::move(board)) {}

template<PColor Color, PKind Kind> void movegen_recurs(Board& board, MoveList& mvList) {
    board.movegen<Color, Kind>(mvList);
}


void BoardState::registerMove(const Move& move) noexcept {
    SG_ASSERT(!move.NAM());

    PKind capturedKind = PKind::None;
    PKind moveKind = PKind::None;

    BB fromMask = 1ull << move.from, toMask = 1ull << move.to;
    auto moveColor = m_board.getColor(fromMask);

    bool isCapture = !m_board.emptyM(toMask) && !move.castling;
    if (move.isEnpass) {
        SQ victimPos = (moveColor == PColor::W ? move.to - 8 : move.to + 8);
        auto [c, k] = m_board.kill(victimPos);
        SG_ASSERT(c != moveColor);
        capturedKind = k;
        moveKind = PKind::pP;
    }
    else if (isCapture) {
        auto [color, kind] = m_board.kill(move.to);
        capturedKind = kind;
    }

    bool promo = false;
    // uint8_t newRPos = 0x00, rPos = 0x00;
    if (move.castling) {
        uint8_t newKPos = CASTL_NEW_KING_POS(move.from, move.castling);
        auto newRPos = CASTL_NEW_ROOK_POS(move.from, move.castling);
        auto rPos = CASTL_ORIG_ROOK_POS(move.from, move.castling);
        m_board.slideTo(move.from, newKPos);
        m_board.slideTo(rPos, newRPos);
        moveKind = PKind::pK;
    }
    else {
        moveKind = m_board.slideTo(move.from, move.to);

        // test for the Promotion
        if(moveKind == PKind::pP && !move.isEnpass) {
            auto col = m_board.getColor(toMask);
            if ((col == PColor::W && toMask & NRank::r8)
                    || (col == PColor::B && toMask & NRank::r1)) {
                promo = true;
                m_board.kill(move.to);
                m_board.put(PKind::pQ, moveColor, move.to);
                m_nonPawnMaterial[col2int(moveColor)] -= PieceScores[static_cast<unsigned>(PKind::pP)];
                m_nonPawnMaterial[col2int(moveColor)] += PieceScores[static_cast<unsigned>(PKind::pQ)];
            }
        }
    }
    m_board.updateKey(moveColor, move.castling, move.isEnpass);

    undo_recs_ rec{};
    rec.data = UNDO_REC_DATA(move.from, move.to, moveKind, false);
    rec.meta = UNDO_REC_RULE_50(m_rule50Ply);
    rec.meta |= UNDO_REC_CAPTURED_KIND(capturedKind);
    rec.meta |= UNDO_REC_CASTL(move.castling);
    rec.meta |= UNDO_REC_PROMO(promo);
    rec.meta |= UNDO_REC_ENPASS(move.isEnpass);

    m_undoList.emplace_back(rec);
    if(!isCapture && moveKind != PKind::pP) m_rule50Ply++;
    else m_rule50Ply = 0;

    if (move.castling || moveKind == PKind::pK) {
        if (moveColor == PColor::W) m_wKingMoves++;
        else m_bKingMoves++;
    }

    if (capturedKind == PKind::pK)
        setKingExistence_(invert(moveColor), false);
    else if (!promo && capturedKind != PKind::None)
        m_nonPawnMaterial[col2int(invert(moveColor))] -= PieceScores[static_cast<unsigned>(capturedKind)];
}

void BoardState::movegen(MoveList& mvList) noexcept {
    details::MG<PColor::W, PKind::pP>{}.run(m_board, mvList, *this);
}


void BoardState::undo() noexcept {
    undo_recs_ rec = m_undoList.back();; m_undoList.pop_back();

    auto from = UNDO_REC_GET_FROM(rec);
    auto to = UNDO_REC_GET_TO(rec);
    auto moveKind = UNDO_REC_GET_MOVE_KIND(rec);
    auto capturedKind = UNDO_REC_GET_CAPTURED_KIND(rec);
    auto rule50 = UNDO_REC_GET_RULE_50(rec);
    auto promo = UNDO_REC_GET_PROMO(rec);
    auto isEnpass = UNDO_REC_GET_ENPASS(rec);
    auto castl = UNDO_REC_GET_CASTL(rec);

    BB fromMask = 1ull << from;
    BB toMask = 1ull << to;
    PColor moveColor;
    if(castl) {
        auto kPos = CASTL_NEW_KING_POS(from, castl);
        auto rPos = CASTL_NEW_ROOK_POS(from, castl);
        auto origRPos = CASTL_ORIG_ROOK_POS(from, castl);
        m_board.slideTo(kPos, from);
        m_board.slideTo(rPos, origRPos);
        moveColor = m_board.getColor(fromMask);
    }
    else {
        moveColor = m_board.getColor(toMask);
        m_board.slideTo(to, from);
        if (capturedKind != PKind::None) {
            if(isEnpass) {
                auto enpassVictimSq = moveColor == PColor::W ? to - 8 : to + 8;
                m_board.put(capturedKind, invert(moveColor), enpassVictimSq);
            }
            else m_board.put(capturedKind, invert(moveColor), to);
        }
        if (promo) {
            // handle only the case P -> Q
            m_board.kill(from);
            m_board.put(PKind::pP, moveColor, from);
            m_nonPawnMaterial[col2int(moveColor)] += PieceScores[static_cast<unsigned>(PKind::pP)];
            m_nonPawnMaterial[col2int(moveColor)] -= PieceScores[static_cast<unsigned>(PKind::pQ)];
        }
    }

    m_board.updateKey(moveColor, castl, isEnpass);
    m_rule50Ply = rule50;
    if (castl || moveKind == PKind::pK) {
        if (moveColor == PColor::W) m_wKingMoves--;
        else m_bKingMoves--;
    }

    if (capturedKind == PKind::pK)
        setKingExistence_(invert(moveColor), true);
    else if (!promo && capturedKind != PKind::None)
        m_nonPawnMaterial[col2int(invert(moveColor))] += PieceScores[static_cast<unsigned>(capturedKind)];
}

std::size_t BoardState::ply() const noexcept {
    return m_undoList.size();
}

bool BoardState::gameover() const noexcept {
    return draw() || checkmate(PColor::W) || checkmate(PColor::B);
}

Score BoardState::nonPawnMaterial(PColor color) const noexcept {
    return m_nonPawnMaterial[col2int(color)];
}

bool BoardState::checkmate(PColor color) const noexcept {
    return color ? !m_wKingExists : !m_bKingExists;
}

void BoardState::setKingExistence_(PColor color, bool exists) noexcept {
    auto& k = color ? m_wKingExists : m_bKingExists;
    k = exists;
}

void BoardState::resetState(unsigned rule50) noexcept {
    {
        decltype(m_undoList) tmp{};
        m_undoList.swap(tmp);
    }
    m_rule50Ply = rule50;
}

BoardState::undoList_t BoardState::history() const noexcept {
    return m_undoList;
}

bool BoardState::draw() const noexcept {
    return m_rule50Ply > 50;
}

} // namespace brd
