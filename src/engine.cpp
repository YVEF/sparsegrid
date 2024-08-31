#include <iostream>
#include "engine.h"
#include "core/CallerThreadExecutor.h"
#include "search/mtdsearch.h"
#include "common/options.h"
#include "uci/fen.h"
#include "dbg/debugger.h"
#include "board/move.h"



namespace sg {

template <typename TExecutor>
Engine<TExecutor>::Engine(common::Options& opts, common::Stat& stat, eval::Evaluator& eval, const uci::Fen& fen, brd::Board&& board) noexcept 
: m_opts(opts), m_stat(stat), m_evalu(eval), m_tm({}), m_state(std::move(board)), m_ttable(m_opts, m_stat),
    m_searcher(search::MtdSearch<TExecutor>{m_opts, m_stat, m_tm, m_ttable, m_evalu}), m_fen(fen) {}


template <typename TExecutor>
void Engine<TExecutor>::initNewGame(PColor color) noexcept {
    m_opts.EngineSide = color;
}

template <typename TExecutor>
void Engine<TExecutor>::setupNewBoard(PColor color) noexcept {
    while(m_state.ply())
        m_state.undo();

    m_opts.EngineSide = color;
}


std::string sq2text(SQ sq) noexcept {
    constexpr char rank[] {'1','2','3','4','5','6','7','8'};
    constexpr char file[] {'a','b','c','d','e','f','g','h'};
    std::string c{file[sq&8]}; c.push_back(rank[sq/8]);
    return c;
}

template <typename TExecutor>
GoResult Engine<TExecutor>::go_() noexcept {
    m_tm.startCounting();
    auto report = m_searcher.pvMove(m_state);
    m_state.registerMove(report.pvMove);
    return {
        sq2text(report.pvMove.from) + sq2text(report.pvMove.to),
        sq2text(report.ponder.from) + sq2text(report.ponder.to)
    };
}

template <typename TExecutor>
search::TimeManager& Engine<TExecutor>::tm() noexcept {
    return m_tm;
}

template <typename TExecutor>
brd::BoardState& Engine<TExecutor>::state() noexcept {
    return m_state;
}

template <typename TExecutor>
void Engine<TExecutor>::printDbg(std::ostream& os) const noexcept {
    Debugger::printBB(m_state);
    os << "Fen: " << m_fen.str(m_state) << "\nKey: " << std::hex << std::uppercase << std::setfill('0') << std::setw(16) << m_state.getBoard().key()
        << std::setfill(' ') << std::dec << "\nCheckers: " << std::endl;
}


template <typename TExecutor>
void Engine<TExecutor>::move(SQ from, SQ to) noexcept {
    auto move = recognizeMove(from, to, m_state.getBoard());
    m_state.registerMove(move);
}




template class Engine<exec::CallerThreadExecutor>;


} // namespace sg

extern "C" {
void printEngineHello(int i) noexcept {
    std::cout << "Hello from " << i << std::endl;
}

}
