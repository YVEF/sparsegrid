#include "common/stat.h"
#include "core/caller_thread_executor.h"
#include "eval/evaluator.h"
#include <iostream>
constexpr int version = 2;
#include "uci/protocol.h"
#include "engine.h"

int main(int argc, char** argv) {
    std::cout << "Welcome to SparseGrid" << version << std::endl;

    common::Options opts{};
    common::Stat stat;
    eval::Evaluator evalu{opts};
    uci::Fen fen;
    brd::Board board;
    sg::Engine<exec::CallerThreadExecutor> engine{opts, stat, evalu, fen, std::move(board)};
    uci::Protocol<exec::CallerThreadExecutor> ucip{engine, std::cin, std::cout, opts, fen};

    ucip.ready();

    return 0;
}
