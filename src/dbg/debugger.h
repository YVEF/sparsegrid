#ifndef INCLUDE_DBG_DEBUGGER_H_
#define INCLUDE_DBG_DEBUGGER_H_

#include "../core/defs.h"
#include <ostream>
namespace brd { class Board; class BoardState; }

class Debugger {
public:
    static void printBB(const brd::Board& board, std::ostream&) noexcept;
    static void printBB(const brd::BoardState& board, std::ostream&) noexcept;
    static void printBB(const brd::Board& board) noexcept;
    static void printBB(const brd::BoardState& board) noexcept;
    static void printBB(BB brd) noexcept;
    static void printNN(const brd::BoardState& state) noexcept;


    static void unwrapHistory(const brd::BoardState& board) noexcept;
};

#endif  // INCLUDE_DBG_DEBUGGER_H_
