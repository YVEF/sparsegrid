#include <chrono>
#include <board/board_state.h>
#include <dbg/debugger.h>

using namespace std::chrono;
#define NOOPT(r) asm ("""":"=r"(r):"r"(r))


static uint64_t moveGenRecursive(brd::BoardState& state, unsigned depth, bool firstPlayer) {
    if(depth <= 0 || state.gameover()) return 0;

    uint64_t result = 0;
    brd::MoveList mvList{};
    if (firstPlayer) state.movegenFor<PColor::W>(mvList);
    else state.movegenFor<PColor::B>(mvList);
    while (mvList.size()) {
        auto move = mvList.pop();
        state.registerMove(move);
        // Debugger::printBB(state);
        moveGenRecursive(state, depth-1, !firstPlayer);
        state.undo();
    }
    return result;
}


void perftGen(unsigned depth) {
    brd::BoardState state(brd::Board{});
    moveGenRecursive(state, depth, true);
}

