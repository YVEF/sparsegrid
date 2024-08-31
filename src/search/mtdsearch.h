#ifndef INCLUDE_SEARCH_MTDSEARCH_H_
#define INCLUDE_SEARCH_MTDSEARCH_H_


#define INF 16639
#define CHECKMATE_EVAL 8447
#define MIN_CHECKMATE_EVAL 8410

#include "../board/move.h"
namespace common { struct Options; class Stat; }
namespace brd { class BoardState; class Board; }
namespace eval { class Evaluator; }

namespace search {
namespace str {
struct Report {
    brd::Move pvMove;
    brd::Move ponder;
    bool ok;
};
} // namespace str

class TimeManager;
class TTable;
namespace detail { struct SearchContext; }
template<typename TExecutor>
class MtdSearch {
public:
    explicit MtdSearch(
            common::Options& opts, common::Stat& stat, 
            TimeManager& tm, TTable& ttable, eval::Evaluator& eval) noexcept;

    [[nodiscard]] search::str::Report pvMove(brd::BoardState& state) noexcept;
    ~MtdSearch() = default;


private:
    common::Options&    m_opts;
    common::Stat&       m_stat;
    TTable&             m_ttable;
    TimeManager&        m_tm;
    eval::Evaluator&    m_eval;
    TExecutor           m_executor;
    // const book*                 m_book;
    // const tracer<TExecutor>*    m_tracer;

    template<bool PV>
    std::pair<Score, brd::Move> AlphaBeta(
        brd::BoardState& state, Score alpha, Score beta, unsigned depth, bool even, detail::SearchContext& ctx) noexcept;

    Score MTDF_(brd::BoardState& state, Score f, unsigned depth, detail::SearchContext&) noexcept;
    Score eval_(brd::BoardState&, const detail::SearchContext&) noexcept;
};


} // namespace search


#endif  // INCLUDE_SEARCH_MTDSEARCH_H_
