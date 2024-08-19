#ifndef INCLUDE_BOARD_BOARD_H_
#define INCLUDE_BOARD_BOARD_H_
#include "../core/defs.h"
#include "movegen.h"
#include <utility>


namespace brd {
#define TOTAL_PKIND_NUM 12u
#define BRD_SIZE 64u

struct MoveList;
class BoardState;

typedef uint64_t BrdKey_t;
class Board {
public:
    explicit Board() noexcept;
    ~Board() noexcept = default;
    Board(const Board&) noexcept = default;
    Board(Board&&) noexcept = default;
    Board& operator=(const Board&) noexcept = delete;
    Board& operator=(Board&&) = delete;

    template<PColor Color, PKind Kind>
    BB getPieceSqMask() const noexcept;

    /*
     * @brief   Move square A -> B
     */
    PKind slideTo(uint8_t from, uint8_t to) noexcept;

    /*
     * @brief   Kill a piece on the square
     */
    std::pair<PColor, PKind> kill(SQ sq) noexcept;

    /*
     * @brief   Check if square is empty (by square number)
     */
    bool empty(SQ sq) const noexcept;

    /*
     * @brief   Check if square is empty (by position mask)
     */
    bool emptyM(BB mask) const noexcept;

    /*
     * @brief   Zobrist hash stamp of the board
     */
    [[nodiscard]] BrdKey_t key() const noexcept;

    /*
     * @brief   Clear the board
     */
    void clear() noexcept;

    template <PColor Color, PKind Kind> void movegen(MoveList& mvList, const BoardState&) const noexcept;
    template<PColor Color> BB attackMap(/* SQ sq */) const noexcept;

    PKind getKind(BB mask) const noexcept;
    PColor getColor(BB mask) const noexcept;
    void put(PKind, PColor, SQ) noexcept;

    /*
     * @brief   Update board data like zobrist key, piece count moves and some misc. counters
     */
    void updateKey(uint8_t castling, bool isEnpass) noexcept;

    uint64_t stateKey() const noexcept;
    BB occupancy() const noexcept;
    void rebuildKey() noexcept;

    [[nodiscard]] std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> getRawBoard() const noexcept;

private:
    BB m_bb_col = 0xFFFF000000000000; // color
    BB m_bb_pbq = 0x2CFF00000000FF2C; // P.B.Q
    BB m_bb_nbk = 0x7600000000000076; // N.B.K
    BB m_bb_rqk = 0x9900000000000099; // R.Q.K
    BrdKey_t m_key = 0;

    /*
     * @brief   Move square A -> B (by position mask)
     */
    PKind slideToM_(BB from, BB to) noexcept;
};


inline BB Board::occupancy() const noexcept {
    return m_bb_rqk | m_bb_pbq | m_bb_nbk;
}


template <PColor Color, PKind Kind>
inline BB attackMap_(BB occ, const brd::Board& board, BB enemyMask) noexcept {
    BB pieceMask = board.getPieceSqMask<Color, Kind>();
    BB r = 0x00;
    while (pieceMask) {
        SQ sq = popLsb(pieceMask);
        auto k = movegen::getPieceOccupancy<Color, Kind>(sq, occ);
        r |= (k & occ & enemyMask);
    }
    return r;
}


template <PColor Color> BB Board::attackMap() const noexcept {
    auto occ = occupancy();
    uint64_t enemyMask = m_bb_col;
    if constexpr (!Color) enemyMask = ~enemyMask;

    BB r = 0x00;
    r = attackMap_<Color, PKind::pB>(occ, *this, enemyMask);
    r |= attackMap_<Color, PKind::pQ>(occ, *this, enemyMask);
    r |= attackMap_<Color, PKind::pR>(occ, *this, enemyMask);
    r |= attackMap_<Color, PKind::pN>(occ, *this, enemyMask);

    return r;
}

} // namespace brd


inline uint64_t flushBit(uint64_t input, int i) noexcept {
    return input & ~(1ULL << i);
}




#endif  // INCLUDE_BOARD_BOARD_H_

