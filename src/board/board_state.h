#ifndef INCLUDE_BOARD_BOARD_STATE_H_
#define INCLUDE_BOARD_BOARD_STATE_H_
#include <deque>
#include <vector>
#include "move.h"
#include "../core/defs.h"
#include "board.h"
#include "../core/scores.h"


#define UNDO_REC_DATA(from, to, kind, isNullMove) \
    static_cast<uint16_t>((from) & 0x3F) \
    | (static_cast<uint16_t>((to) & 0x3F) << 0x06) \
    | (static_cast<uint16_t>(((int)kind) & 0x07) << 0x0C) \
    | static_cast<uint16_t>((isNullMove) ? 0x8000 : 0x00)

#define UNDO_REC_RULE_50(rule) static_cast<uint16_t>((rule) & 0x3F)
#define UNDO_REC_CAPTURED_KIND(kind) static_cast<uint16_t>((kind) & 0x07) << 6
#define UNDO_REC_CASTL(castl) static_cast<uint16_t>((castl) & 0x03) << 13
#define UNDO_REC_PROMO(promo) static_cast<uint16_t>((uint16_t)(promo) & 0x01) << 15
#define UNDO_REC_ENPASS(enpass) static_cast<uint16_t>((uint8_t)(bool)(enpass)) << 9;


#define UNDO_REC_GET_FROM(mv) static_cast<uint8_t>((mv).data & 0x3f)
#define UNDO_REC_GET_TO(mv) static_cast<uint8_t>(((mv).data >> 0x06) & 0x3f)
#define UNDO_REC_GET_MOVE_KIND(mv) static_cast<PKind>(((mv).data >> 0x0C) & 0x07)
#define UNDO_REC_GET_CAPTURED_KIND(mv) static_cast<PKind>(((mv).meta >> 6) & 0x07)
#define UNDO_REC_GET_RULE_50(mv) static_cast<uint8_t>((mv).meta & 0x3F)
#define UNDO_REC_GET_PROMO(mv) static_cast<bool>(((mv).meta >> 15) & 0x01)
#define UNDO_REC_GET_ENPASS(mv) static_cast<bool>(((mv).meta >> 9) & 0x01)
#define UNDO_REC_GET_CASTL(mv) static_cast<uint8_t>(((mv).meta >> 13) & 0x03)



namespace brd {
class BoardState {
/** Undo move records. 32 bits packed */
struct undo_recs_ {
    /** layout:
     * |0|000 |0000 00|00 0000
     * | |    |       ---------- from
     * | |    ------------------ to
     * | ----------------------- kind
     * ------------------------- is null move
     */
    uint16_t data;

    /** layout:
     * |0|00|0 00|0|0 00|00 0000
     * | |  |    |  |    -------- rule_50_ply // The amount of non attack move before the current one
     * | |  |    |  ------------- taken kind
     * | |  |    ---------------- is_enpass // Indicated that the move is enpassant
     * | |  --------------------- reserved
     * | ------------------------ castling // The castling tipe (00, 01, 10) = (none, short, long)
     * -------------------------- p2q // Indicates that a pawn that was replaced with a queen
     */
    uint16_t meta;
};

public:
    typedef std::deque<undo_recs_> undoList_t;
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
    PColor getSideToMove() const noexcept;
    void setSideToMove(PColor color) noexcept;
    const undo_recs_& getLastMove() const noexcept;
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
    template<PColor Color> bool castlPossible() const noexcept;
    template<PColor Color> bool kingUnderCheck() const noexcept;

    undoList_t history() const noexcept;

private:
    brd::Board m_board;

    /** Move records list for undo operations and previous move analyzing */
    mutable undoList_t  m_undoList;
    uint8_t             m_rule50Ply = 0;
    unsigned int        m_wKingMoves = 0, m_bKingMoves = 0;
    BB                  m_kingAttackers[2] = {0,0};
    Score               m_nonPawnMaterial[2] = {INIT_MATERIAL, INIT_MATERIAL};
    unsigned            m_wKingExists = true, m_bKingExists = true;

    void setKingExistence_(PColor, bool) noexcept;
};

inline const BoardState::undo_recs_& BoardState::getLastMove() const noexcept {
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

template<PColor Color> bool BoardState::castlPossible() const noexcept {
    if constexpr (Color == PColor::W) {
        return !m_wKingMoves;
    }
    else {
        return !m_bKingMoves;
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




} // namespace brd
#endif  // INCLUDE_BOARD_BOARD_STATE_H_
