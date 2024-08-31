#ifndef INCLUDE_BOARD_MOVE_H_
#define INCLUDE_BOARD_MOVE_H_

#include <cstdint>
#include <ostream>
#include "../core/defs.h"

namespace brd {
// todo: think about expediency
struct MoveMask {
    SQ from = 0x00;
    QMoves qmoves = 0x00;
    AMoves amoves = 0x00;
    /* 1 - short, 2 - long */
    uint8_t castling : 2 = 0x00;
    /* 1 - left, 2 - rigth */
    uint8_t enpassant : 6 = 0x00;
    // uint8_t promo: 1;
};

struct Move {
    // 0|0|00 |0000 00|00 0000
    //  | |   |       --------- from (6)
    //  | |   ----------------- to (6)
    //  | --------------------- castling type (2)
    //  ----------------------- is enpassant (1)
    uint16_t from : 6;
    uint16_t to : 6;
    uint16_t castling : 2;
    uint16_t isEnpass : 1;
    uint16_t isNull : 1;

    /** is NOT A MOVE */
    bool NAM() const noexcept { return !from && !to && !castling && !isEnpass; }

    friend std::ostream& operator<<(std::ostream&, const Move&) noexcept;

    bool operator==(const Move& rhs) const noexcept;

    static Move None() { return brd::Move{}; }

    friend std::ostream& operator<<(std::ostream& str, const Move& move) noexcept;
};
static_assert(sizeof(brd::Move) == 2, "brd::Move packed into 16 bits");

#define NONE_MOVE brd::Move::None()


inline std::ostream& operator<<(std::ostream& str, const Move& move) noexcept {
    str << (int)move.from << '/' << (int)move.to << '/' 
        << (int)move.castling << '/' << (int)move.isEnpass;
    return str;
}

inline bool Move::operator==(const Move& rhs) const noexcept {
    return from == rhs.from && to == rhs.to
           && isEnpass == rhs.isEnpass
           && castling == rhs.castling;
}


enum CastlingType : uint8_t {
    C_NONE = 0x00,
    C_SHORT = 0x01,
    C_LONG = 0x02
};


inline CastlingType operator|(CastlingType l, CastlingType r) noexcept {
    return static_cast<CastlingType>((uint8_t)l | (uint8_t)r);
}

inline CastlingType operator&(CastlingType l, CastlingType r) noexcept {
    return static_cast<CastlingType>((uint8_t)l & (uint8_t)r);
}


Move mkMove(SQ from, SQ to) noexcept;
Move mkEnpass(SQ from, SQ to) noexcept;
Move mkCastling(SQ from, CastlingType ct) noexcept;

struct MoveList {
    constexpr static std::size_t capacity = 128;
    void push(Move move) noexcept;
    Move pop() noexcept;
    std::size_t size() const noexcept;
    const Move& operator[](std::size_t i) noexcept;

private:
    std::size_t m_ptr = 0;
    Move        m_data[capacity];
};

inline std::size_t MoveList::size() const noexcept {
    return m_ptr;
}

inline const Move& MoveList::operator[](std::size_t i) noexcept {
    return m_data[i];
}

class Board;
brd::Move recognizeMove(SQ from, SQ to, const brd::Board&) noexcept;

} // namespace brd



#endif  // INCLUDE_BOARD_MOVE_H_
