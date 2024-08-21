#include <iostream>
#include "interop.h"

interop::CDC::CDC(brd::BoardState&& state, common::Options&& opts) noexcept
: m_state(std::move(state)), m_opts(opts) {}

std::size_t interop::MoveCollection::size() noexcept {
    return m_size;
}

CDCMove interop::MoveCollection::getMove(int i) noexcept {
    return m_data[i];
}

void interop::MoveCollection::reset() noexcept {
    std::cout << "reset\n";
    m_size = 0;
    std::cout << "reset done...\n";
}

void interop::MoveCollection::push(CDCMove move) noexcept {
    m_data[m_size++] = move;
}



extern "C" {
interop::CDC* initCDC(bool color) noexcept {
    common::Options opts{};
    opts.EngineSide = static_cast<PColor>(color);
    return new interop::CDC(brd::BoardState{brd::Board{}}, std::move(opts));
}


void makeMove(interop::CDC* cdc, const brd::Move& move) noexcept {
    cdc->m_state.registerMove(move);
}

void undoMove(interop::CDC* cdc) noexcept {
    cdc->m_state.undo();
}

void freeCDC(interop::CDC* cdc) noexcept {
    delete cdc;
}

interop::MoveCollection* nextMoves(interop::CDC* cdc, bool color) noexcept {
    cdc->m_mvCollection.reset();
    std::cout << "reset done\n";
    int a = 4;
    if (a == 4) return nullptr;
    if (a) return &cdc->m_mvCollection;

    brd::MoveList mvlist{};
    if (color) cdc->m_state.movegenFor<PColor::W>(mvlist);
    else cdc->m_state.movegenFor<PColor::B>(mvlist);

    while (mvlist.size()) {
        auto move = mvlist.pop();
        CDCMove cdcMove{};
        cdcMove.from = move.from;
        cdcMove.to = move.to;
        cdcMove.castling = move.castling;
        cdcMove.isEnpassant = move.isEnpass;
        cdc->m_mvCollection.push(cdcMove);
    }

    return &cdc->m_mvCollection;
}

void welcome(interop::CDC*) {
    std::cout << "hi hi\n";
}
} // extern C

