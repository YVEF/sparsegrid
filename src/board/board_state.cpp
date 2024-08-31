#include <optional>
#include "board_state.h"
#include "../dbg/sg_assert.h"
#include "../core/scores.h"
#include "../dbg/debugger.h"
#include "../common/options.h"

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
: m_board(board) {}

BoardState::BoardState(const brd::BoardState& rhs) noexcept
: m_board(rhs.m_board), m_undoList(rhs.m_undoList), m_rule50Ply(rhs.m_rule50Ply), m_wKingMoves(rhs.m_wKingMoves),
m_bKingMoves(rhs.m_bKingMoves), m_wKingExists(rhs.m_wKingExists), m_bKingExists(rhs.m_bKingExists),
m_lwRp(rhs.m_lwRp), m_lbRp(rhs.m_lbRp), m_lwRMoves(rhs.m_lwRMoves), m_rwRMoves(rhs.m_rwRMoves),
m_lbRMoves(rhs.m_lbRMoves), m_rbRMoves(rhs.m_rbRMoves), m_fenEnpassMove(rhs.m_fenEnpassMove),
m_fenNextPlayer(rhs.m_fenNextPlayer), m_buildFromFen(rhs.m_buildFromFen), m_fenCastlingMask(rhs.m_fenCastlingMask)
{
    m_nonPawnMaterial[0] = rhs.m_nonPawnMaterial[0];
    m_nonPawnMaterial[1] = rhs.m_nonPawnMaterial[1];
}

BoardState::BoardState(brd::BoardState&& rhs) noexcept
: m_board(rhs.m_board), m_undoList(std::move(rhs.m_undoList)), m_rule50Ply(rhs.m_rule50Ply),
  m_wKingMoves(rhs.m_wKingMoves), m_bKingMoves(rhs.m_bKingMoves), m_wKingExists(rhs.m_wKingExists),
  m_bKingExists(rhs.m_bKingExists),
  m_lwRp(rhs.m_lwRp), m_lbRp(rhs.m_lbRp), m_lwRMoves(rhs.m_lwRMoves), m_rwRMoves(rhs.m_rwRMoves),
  m_lbRMoves(rhs.m_lbRMoves), m_rbRMoves(rhs.m_rbRMoves), m_fenEnpassMove(rhs.m_fenEnpassMove),
  m_fenNextPlayer(rhs.m_fenNextPlayer), m_buildFromFen(rhs.m_buildFromFen), m_fenCastlingMask(rhs.m_fenCastlingMask)
{
    m_nonPawnMaterial[0] = rhs.m_nonPawnMaterial[0];
    m_nonPawnMaterial[1] = rhs.m_nonPawnMaterial[1];
}

void BoardState::updateRookMeta_(PColor color, SQ from, SQ to, bool inc) noexcept {
    int coeff = inc ? 1 : -1;
    if (color) {
        if (from == m_lwRp) {
            m_lwRp = to;
            m_lwRMoves += coeff;
        }
        else m_rwRMoves += coeff;
    }
    else {
        if (from == m_lbRp) {
            m_lbRp = to;
            m_lbRMoves += coeff;
        }
        else m_rbRMoves += coeff;
    }
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
     uint8_t newRPos = 0x00, rPos = 0x00;
    if (move.castling) {
        uint8_t newKPos = CASTL_NEW_KING_POS(move.from, move.castling);
        newRPos = CASTL_NEW_ROOK_POS(move.from, move.castling);
        rPos = CASTL_ORIG_ROOK_POS(move.from, move.castling);
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
    m_board.updateKey(move.castling, move.isEnpass);

    undoRec_ rec = buildUndoRec_(move, moveKind, capturedKind, promo);
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

    // update dedicated rook position vars for castle handling
    if (moveKind == PKind::pR)
        updateRookMeta_(moveColor, move.from, move.to, true);
    else if (move.castling)
        updateRookMeta_(moveColor, rPos, newRPos, true);

    FenResetState();
}

void BoardState::movegen(MoveList& mvList) noexcept {
    if (gameover()) return;
    details::MG<PColor::W, PKind::pP>{}.run(m_board, mvList, *this);
}


void BoardState::undo() noexcept {
    undoRec_ rec = m_undoList.back();; m_undoList.pop_back();

    auto from = static_cast<SQ>(rec.from);
    auto to = static_cast<SQ>(rec.to);
    auto moveKind = static_cast<PKind>(rec.moveKind);
    auto capturedKind = static_cast<PKind>(rec.capturedKind);
    auto rule50 = static_cast<uint8_t>(rec.rule50ply);
    auto promo = static_cast<bool>(rec.promo);
    auto isEnpass = static_cast<bool>(rec.isEnpass);
    auto castle = static_cast<uint8_t>(rec.castling);

    BB fromMask = 1ull << from;
    BB toMask = 1ull << to;
    PColor moveColor;
    SQ rPos = 0x00, origRPos = 0x00;
    if(castle) {
        auto kPos = CASTL_NEW_KING_POS(from, castle);
        rPos = CASTL_NEW_ROOK_POS(from, castle);
        origRPos = CASTL_ORIG_ROOK_POS(from, castle);
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

    m_board.updateKey(castle, isEnpass);
    m_rule50Ply = rule50;
    if (castle || moveKind == PKind::pK) {
        if (moveColor == PColor::W) m_wKingMoves--;
        else m_bKingMoves--;
    }

    if (capturedKind == PKind::pK)
        setKingExistence_(invert(moveColor), true);
    else if (!promo && capturedKind != PKind::None)
        m_nonPawnMaterial[col2int(invert(moveColor))] += PieceScores[static_cast<unsigned>(capturedKind)];

    if (moveKind == PKind::pR)
        updateRookMeta_(moveColor, to, from, false);
    else if (castle)
        updateRookMeta_(moveColor, rPos, origRPos, false);

    FenSetEnpass(0x00);
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



unsigned BoardState::PG_possibleCastlMask() const noexcept {
    unsigned mask = 0;
    if (kindNotMoved<PColor::W>()) {
        if (rightRookNotMoved<PColor::W>()) mask |= 0x01;
        if (leftRookNotMoved<PColor::W>()) mask |= 0x02;
    }

    if (kindNotMoved<PColor::B>()) {
        if (rightRookNotMoved<PColor::B>()) mask |= 0x04;
        if (leftRookNotMoved<PColor::B>()) mask |= 0x08;
    }

    return mask;
}

bool BoardState::validateEnpassPosition(SQ enpassSQ, SQ pawnPos) const noexcept {
    BB pawnMask = 1ull << pawnPos;
    PColor pawnColor = m_board.getColor(pawnMask);

    if (!m_board.empty(enpassSQ)) return 0x00;
    BB toLeftMask = 1ull << (pawnPos-1);
    auto leftKind = m_board.getKind(toLeftMask);
    if (!(pawnMask & NFile::fA)
        && (leftKind == None || (leftKind == PKind::pP && pawnColor != m_board.getColor(toLeftMask))))
        return true;

    BB toRightMask = 1ull << (pawnPos+1);
    auto rightKind = m_board.getKind(toRightMask);
    if (!(pawnMask & NFile::fH)
        && (rightKind == None || (rightKind == PKind::pP && pawnColor != m_board.getColor(toRightMask))))
        return true;

    return false;
}

SQ BoardState::PG_enpassPos() const noexcept {
    if(!ply()) return 0;

    auto&& prevMove = m_undoList.back();
    auto kind = static_cast<PKind>(prevMove.moveKind);
    if (kind != PKind::pP) [[likely]] return 0x00;
    SQ from = static_cast<SQ>(prevMove.from);
    SQ to = static_cast<SQ>(prevMove.to);
    if (dist(from, to) != 16) return 0x00;


    BB toMask = 1ull << to;
    PColor moveColor = m_board.getColor(toMask);

    BB mask = moveColor ? m_board.getPieceSqMask<PColor::B, PKind::pP>() : m_board.getPieceSqMask<PColor::W, PKind::pP>();
    while (mask) {
        SQ sq = popLsb(mask);
        auto rr = moveColor ?
            movegen::getEnpassantAttack<PColor::B>(sq, *this)
            : movegen::getEnpassantAttack<PColor::W>(sq, *this);
        if (rr) return rr;
    }
    return 0x00;
}


SQ BoardState::FenGetEnpass() const noexcept {
    return m_fenEnpassMove;
}

void BoardState::FenSetEnpass(SQ sq) noexcept {
    m_fenEnpassMove = sq;
}

std::optional<PColor> BoardState::FenGetNextPlayer() const noexcept {
    return m_fenNextPlayer;
}

void BoardState::FenSetNextPlayer(PColor color) noexcept {
    m_fenNextPlayer = color;
}

void BoardState::FenResetState() noexcept {
    m_fenEnpassMove = 0x00;
    m_fenNextPlayer.reset();
    m_buildFromFen = false;
    m_fenCastlingMask = 0x00;
}

bool BoardState::buildFromFen() const noexcept {
    return m_buildFromFen;
}

void BoardState::markBuildFromFen() noexcept {
    m_buildFromFen = true;
}

void BoardState::setFenCastlingMask(uint64_t mask) noexcept {
    m_fenCastlingMask |= mask;
}

uint64_t BoardState::getFenCastlingMask() const noexcept {
    return m_fenCastlingMask;
}

BoardState::undoRec_ BoardState::buildUndoRec_(
    const brd::Move& move, PKind moveKind,
    PKind capturedKind, bool promo) noexcept {

    undoRec_ rec{};
    rec.from = move.from;
    rec.to = move.to;
    rec.moveKind = moveKind;
    rec.isNull = move.isNull;
    rec.rule50ply = m_rule50Ply;
    rec.capturedKind = capturedKind;
    rec.castling = move.castling;
    rec.promo = promo;
    rec.isEnpass = move.isEnpass;
    return rec;
}

PColor getNextPlayerColor(const brd::BoardState& state, const common::Options& opts) noexcept {
    if (state.buildFromFen()) {
        SG_ASSERT(state.FenGetNextPlayer().has_value());
        return state.FenGetNextPlayer().value();
    }

    return (opts.EngineSide && state.ply() % 2 == 0) || (!opts.EngineSide && state.ply() % 2)
           ? PColor::W : PColor::B;
}

} // namespace brd
