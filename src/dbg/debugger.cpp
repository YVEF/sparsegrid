#include "debugger.h"
#include "../core/defs.h"
#include <iostream>
#include "../board/board.h"
#include "../board/board_state.h"



void Debugger::printBB(BB brd) noexcept {
    for(int sq = 56;sq >= 0;sq++) {
        std::cout << ((1ULL << sq) & brd ? '1' : '0') << " ";
        if(sq % 8 == 7) {
            std::cout << '\n';
            sq -= 16;
        }
    }
    std::cout << std::endl;
}

void Debugger::printBB(const brd::Board& board) noexcept {
    printBB(board, std::cout);
}

void Debugger::printBB(const brd::Board& board, std::ostream& os) noexcept {
    for(int sq = 56; sq >= 0; sq++) {
        BB mask = 1ull << sq;
        PKind kind = board.getKind(mask);
        PColor color = board.getColor(mask);
        os << p2c(kind, color) << " ";
        if (sq % 8 == 7) {
            os << '\n';
            sq -= 16;
        }
    }
    os << std::endl;
}



void Debugger::printBB(const brd::BoardState& state) noexcept {
    printBB(state.getBoard(), std::cout);
}

void Debugger::printBB(const brd::BoardState& state, std::ostream& os) noexcept {
    printBB(state.getBoard(), os);
}


void Debugger::unwrapHistory(const brd::BoardState& state) noexcept {
    auto undoList = state.history();
    for (auto& rec : undoList) {
        auto from = static_cast<uint8_t>(rec.from);
        auto to = static_cast<uint8_t>(rec.to);
        auto moveKind = static_cast<PKind>(rec.moveKind);
        auto capturedKind = static_cast<PKind>(rec.capturedKind);
                // auto rule50 = UNDO_REC_GET_RULE_50(rec);
                // auto promo = UNDO_REC_GET_PROMO(rec);
        auto enpass = static_cast<bool>(rec.isEnpass);
                // auto castl = UNDO_REC_GET_CASTL(rec);

        std::cout << "from:" << (int)from << " to:" << (int)to << " moveKind:" << (int)moveKind
            << " capturedKind:" << (int)capturedKind << " enpass:" << (bool)enpass << std::endl;
    }
}






