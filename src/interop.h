#ifndef SPARSEGRID_INTEROP_H
#define SPARSEGRID_INTEROP_H
#include "board/board_state.h"
#include "common/options.h"

extern "C" {
struct CDCMove {
    uint8_t from;
    uint8_t to;
    uint8_t castling;
    bool isEnpassant;
    bool isNAM() { return !from && !to && !castling; }
};
}

namespace interop {
class MoveCollection {
public:
    std::size_t size() noexcept;
    CDCMove getMove(int i) noexcept;
    void reset() noexcept;
    void push(CDCMove) noexcept;
private:
    int m_size = 0;
    CDCMove m_data[64];
};

/*
 * Cross domain communicator
 */
class CDC {
public:
    explicit CDC(brd::BoardState&& state, common::Options&& opts) noexcept;
public:
    brd::BoardState m_state;
    common::Options m_opts;
    interop::MoveCollection m_mvCollection{};
};


} // namespace interop


extern "C" {
    interop::CDC* initCDC(bool color) noexcept;
    void makeMove(interop::CDC* cdc, const brd::Move& move) noexcept;
    void undoMove(interop::CDC* cdc) noexcept;
    void freeCDC(interop::CDC*) noexcept;
    interop::MoveCollection* nextMoves(interop::CDC* cdc, bool color) noexcept;
    void welcome(interop::CDC*);
}

#endif //SPARSEGRID_INTEROP_H
