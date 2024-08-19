#ifndef INCLUDE_SRC_ENGINE_H_
#define INCLUDE_SRC_ENGINE_H_

#include "core/defs.h"
#include "board/board_state.h"
#include "search/tm.h"
#include "common/options.h"
#include "search/tt.h"
#include <functional>
#include <string>
#include <thread>
#include "search/mtdsearch.h"

namespace search { template<typename> class MtdSearch; }
namespace common { class Stat; }
namespace eval { class Evaluator; }
namespace uci { class Fen; }

namespace sg {
struct GoResult {
    std::string bestmove;
    std::string ponder;
};


template<typename TExecutor>
class Engine {
public:
    explicit Engine(common::Options& opts, common::Stat& stat, eval::Evaluator& eval, const uci::Fen& fen, brd::Board&& board) noexcept;

    void move(SQ from, SQ to) noexcept;
    template<typename TGoCallback>
    requires requires (TGoCallback&& t) { { std::invoke(std::forward<TGoCallback>(t), GoResult{}) }; }
    void go(uint64_t wtime, uint64_t btime, TGoCallback&& callback) noexcept;
    void initNewGame(PColor color) noexcept;
    void setupNewBoard(PColor color) noexcept;
    search::TimeManager& tm() noexcept;
    brd::BoardState& state() noexcept;
    void printDbg(std::ostream&) const noexcept;


private:
    common::Options&                m_opts;
    common::Stat&                   m_stat;
    eval::Evaluator&                m_evalu;
    search::TimeManager             m_tm;
    brd::BoardState                 m_state;
    search::TTable                  m_ttable;
    search::MtdSearch<TExecutor>    m_searcher;
    const uci::Fen&                 m_fen;

    GoResult go_() noexcept;
};

template <typename TExecutor> 
template <typename TGoCallback>
requires requires (TGoCallback&& t) { { std::invoke(std::forward<TGoCallback>(t), GoResult{}) }; }
void Engine<TExecutor>::go(uint64_t wtime, uint64_t btime, TGoCallback&& callback) noexcept {
    auto time = m_opts.EngineSide == PColor::W ? wtime : btime;
    m_tm.setTimeout(time / (70-m_state.ply()));

    std::thread thr{[this, cl = std::forward<TGoCallback>(callback)](){
        // auto res = go_();
        std::invoke(cl, go_());
    }};
    thr.detach();
}
} // namespace sg



#endif  // INCLUDE_SRC_ENGINE_H_
