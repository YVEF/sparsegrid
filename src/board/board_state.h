#ifndef INCLUDE_BOARD_BOARD_STATE_H_
#define INCLUDE_BOARD_BOARD_STATE_H_
#include <deque>
#include <vector>
#include <optional>
#include "move.h"
#include "../core/defs.h"
#include "board.h"
#include "../core/scores.h"

namespace common { struct Options; }
namespace brd {
class BoardState {
/** Undo move records. 32 bits packed */
struct undoRec_ {
    uint16_t from : 6;
    uint16_t to : 6;
    uint16_t moveKind : 3;
    uint16_t isNull : 1;

    uint16_t rule50ply : 6;
    uint16_t capturedKind : 3;
    uint16_t isEnpass : 1;
    uint16_t reserved : 3;
    uint16_t castling : 2;
    uint16_t promo : 1;
};
static_assert(sizeof(undoRec_) == 4, "undoRec_ packed into 32 bits");

public:
    typedef std::deque<undoRec_> undoList_t;
    explicit BoardState(brd::Board&& board) noexcept;
    void movegen(MoveList& mvList) noexcept;
    template<PColor Color, PKind Kind> void movegenFor(MoveList& mvList) noexcept;
    template<PColor Color> void movegenFor(MoveList& mvList) const noexcept;
    void registerMove(const Move&) noexcept;

    void undo() noexcept;

    /*
     * @brief   Return non-mutable board
     */
    const Board& getBoard() const noexcept;

    /*
     * @brief   Return mutable board
     */
    Board& getBoardMutable() noexcept;
    const undoRec_& getLastMove() const noexcept;
    std::size_t ply() const noexcept;
    bool gameover() const noexcept;
    bool draw() const noexcept;

    /*
     * @brief   Indicates the checkmate for the color side
     */
    bool checkmate(PColor) const noexcept;

    /*
     * @brief   Non pawn material
     *          It also includes a pawn material at the moment
     */
    Score nonPawnMaterial(PColor) const noexcept;

    void resetState(unsigned rule50) noexcept;

    /*
     * @brief   Indicates that the castling still possible (even if the king under check)
     */
    template<PColor Color> bool kindNotMoved() const noexcept;
    template<PColor Color> bool leftRookNotMoved() const noexcept;
    template<PColor Color> bool rightRookNotMoved() const noexcept;
    template<PColor Color> bool kingUnderCheck() const noexcept;

    undoList_t history() const noexcept;

    // ========= PG, FEN =========
    unsigned PG_possibleCastlMask() const noexcept;
    SQ PG_enpassPos() const noexcept;
    SQ FenGetEnpass() const noexcept;
    std::optional<PColor> FenGetNextPlayer() const noexcept;
    void FenSetNextPlayer(PColor) noexcept;
    void FenSetEnpass(SQ sq) noexcept;
    void FenResetState() noexcept;
    bool validateEnpassPosition(SQ enpassSQ, SQ pawnPos) const noexcept;
    bool buildFromFen() const noexcept;
    void markBuildFromFen() noexcept;
    uint64_t getFenCastlingMask() const noexcept;
    void setFenCastlingMask(uint64_t) noexcept;
    // ========= PG, FEN =========

private:
    brd::Board m_board;

    /** Move records list for undo operations and previous move analyzing */
    mutable undoList_t      m_undoList;
    uint8_t                 m_rule50Ply = 0;
    unsigned int            m_wKingMoves = 0, m_bKingMoves = 0;
//    BB                      m_kingAttackers[2] = {0,0};
    Score                   m_nonPawnMaterial[2] = {INIT_MATERIAL, INIT_MATERIAL};
    unsigned                m_wKingExists = true, m_bKingExists = true;
    SQ                      m_lwRp = SqNum::sqn_a1, m_lbRp = SqNum::sqn_a8; // left black and white rook positions
    unsigned int            m_lwRMoves=0, m_rwRMoves=0, m_lbRMoves=0, m_rbRMoves=0; // left/right black/white rook moves count
    SQ                      m_fenEnpassMove=0;
    std::optional<PColor>   m_fenNextPlayer;
    bool                    m_buildFromFen = false;
    uint64_t                m_fenCastlingMask = 0x00;


    void setKingExistence_(PColor, bool) noexcept;
    void updateRookMeta_(PColor color, SQ from, SQ to, bool inc) noexcept;
    undoRec_ buildUndoRec_(const brd::Move& move, PKind moveKind,
                           PKind capturedKind, bool promo) noexcept;
};

inline const BoardState::undoRec_& BoardState::getLastMove() const noexcept {
    return m_undoList.back();
}

template <PColor Color, PKind Kind>
void BoardState::movegenFor(MoveList& mvList) noexcept {
    m_board.movegen<Color, Kind>(mvList, *this);
}

template <PColor Color>
void BoardState::movegenFor(MoveList& mvList) const noexcept {
    m_board.movegen<Color, PKind::pK>(mvList, *this);
    m_board.movegen<Color, PKind::pB>(mvList, *this);
    m_board.movegen<Color, PKind::pQ>(mvList, *this);
    m_board.movegen<Color, PKind::pP>(mvList, *this);
    m_board.movegen<Color, PKind::pN>(mvList, *this);
    m_board.movegen<Color, PKind::pR>(mvList, *this);
}

inline const Board& BoardState::getBoard() const noexcept {
    return m_board;
}

inline Board& BoardState::getBoardMutable() noexcept {
    return m_board;
}

template<PColor Color> bool BoardState::kindNotMoved() const noexcept {
    if constexpr (Color == PColor::W) {
        return !m_wKingMoves;
    }
    else {
        return !m_bKingMoves;
    }
}

template<PColor Color> bool BoardState::leftRookNotMoved() const noexcept {
    if constexpr (Color == PColor::W) {
        return !m_lwRMoves;
    }
    else {
        return !m_lbRMoves;
    }
}

template<PColor Color> bool BoardState::rightRookNotMoved() const noexcept {
    if constexpr (Color == PColor::W) {
        return !m_rwRMoves;
    }
    else {
        return !m_rbRMoves;
    }
}

template<PColor Color> bool BoardState::kingUnderCheck() const noexcept {
    if constexpr (Color) {
        auto kingMask = m_board.getPieceSqMask<PColor::W, PKind::pK>();
        return kingMask & m_board.attackMap<PColor::B>();
    }
    else {
        auto kingMask = m_board.getPieceSqMask<PColor::B, PKind::pK>();
        return kingMask & m_board.attackMap<PColor::W>();
    }
}


PColor getNextPlayerColor(const brd::BoardState& state, const common::Options& opts) noexcept;



} // namespace brd
#endif  // INCLUDE_BOARD_BOARD_STATE_H_
