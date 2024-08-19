#include "mtdsearch.h"
#include "../board/board_state.h"
#include "tm.h"
#include "../common/options.h"
#include "tt.h"
#include "../dbg/sg_assert.h"
#include "../core/caller_thread_executor.h"
#include "../dbg/debugger.h"
#include "../eval/evaluator.h"
#include "../common/stat.h"


namespace search {
namespace detail {

struct SearchContext {
    int relPly = -1;
    constexpr static unsigned scMaxPly = 64;
    unsigned TDepth = 0;
    brd::Move T1[scMaxPly][scMaxPly];
    bool pvWasFound[scMaxPly];
    void incrementLevel() { relPly++; pvWasFound[relPly] = false; }
    void decrementLevel() { relPly--; }
    void markPvWasFound() { pvWasFound[relPly] = true; }
    bool prevLevelPvFound() { return pvWasFound[relPly+1]; }
};
} // namespace detail


static inline auto movegen(bool isEven, const brd::BoardState& state, PColor searchRootColor) {
    brd::MoveList mvList{};
    if ((isEven && searchRootColor == PColor::W) || (!isEven && searchRootColor == PColor::B))
        state.movegenFor<PColor::W>(mvList);
    else
        state.movegenFor<PColor::B>(mvList);
    return mvList;
}


template <typename TExecutor>
MtdSearch<TExecutor>::MtdSearch(common::Options& opts, common::Stat& stat, 
        TimeManager& tm, TTable& ttable, eval::Evaluator& eval) noexcept 
: m_opts(opts), m_stat(stat), m_ttable(ttable), m_tm(tm), m_eval(eval) {}

// todo: fix PVLine
template <typename TExecutor>
search::str::Report MtdSearch<TExecutor>::pvMove(brd::BoardState& state) noexcept {
    Score f1 = 0, f2 = 0;
    unsigned depth = 1;
    detail::SearchContext ctx{};
    m_ttable.incrementAge();

    for (; depth <= m_opts.MaxDepthPly && f1 < MIN_CHECKMATE_EVAL && f1 > -MIN_CHECKMATE_EVAL && !m_tm.timeout(); depth++) {
        ctx.TDepth = depth;
        if (depth % 2) 
            f1 = MTDF_(state, f1, depth, ctx);
        else
            f2 = MTDF_(state, f2, depth, ctx);
    }

    SG_ASSERT(!ctx.T1[0][0].NAM());

    search::str::Report report{};
    report.ok = true;
    report.pvMove = ctx.T1[0][0];
    report.ponder = ctx.T1[0][1];

    m_stat.resetSingleSearch();
    return report;
}



template <typename TExecutor>
Score MtdSearch<TExecutor>::MTDF_(brd::BoardState& state, int16_t f, unsigned depth, detail::SearchContext& ctx) noexcept {
    int16_t lowerBound = -INF, upperBound = INF, beta = 0;
    while (lowerBound < upperBound && !m_tm.timeout()) {
        beta = (f == lowerBound) ? f+1 : f;
        if (depth <= 3) {
            auto [l, _] = AlphaBeta<false>(state, beta-1, beta, depth, true, ctx);
            f = l;
        }
        else {
            auto [l, _] = AlphaBeta<true>(state, beta-1, beta, depth, true, ctx);
            f = l;
            beta = (f == lowerBound) ? f+1 : f;
            auto [l2, m] = AlphaBeta<false>(state, beta-1, beta, depth, true, ctx);
            f = l2;
        }

        if (f < beta) upperBound = f;
        else lowerBound = f;
    }
    return f;
}


static Score checkmateScore(const brd::BoardState& state, PColor engineColor, const detail::SearchContext& ctx) noexcept {
    return state.checkmate(engineColor) ?
        static_cast<Score>(-CHECKMATE_EVAL + ctx.relPly)
        : static_cast<Score>(CHECKMATE_EVAL - ctx.relPly);
}


static void copyPV(detail::SearchContext& ctx, const brd::Move& best, const brd::Move& prevBest) noexcept {
    ctx.T1[ctx.relPly][0] = best;
    if (ctx.prevLevelPvFound()) {
        for(int i=0; !ctx.T1[ctx.relPly+1][i].NAM(); i++)
            ctx.T1[ctx.relPly][i+1] = ctx.T1[ctx.relPly+1][i];
    }
    ctx.markPvWasFound();

    if (prevBest.NAM()) return;
    ctx.T1[ctx.relPly][1] = prevBest;
}



template <typename TExecutor>
template<bool PV>
std::pair<Score, brd::Move> MtdSearch<TExecutor>::AlphaBeta(
        brd::BoardState& state, Score alpha, Score beta, unsigned depth,
        bool even, detail::SearchContext& ctx) noexcept {

    if (state.gameover()) {
        if (state.draw()) return {0x00, NONE_MOVE};
        return {checkmateScore(state, m_opts.EngineSide, ctx), NONE_MOVE};
    }

    ctx.incrementLevel();
    auto origAlpha = alpha;
    auto origBeta = beta;

    TTDescriptor ttdesc = m_ttable.probe(state.getBoard().key());
    if (ttdesc.hit()) {
        auto entry = ttdesc.entry();
        if (entry->horizon > depth) {
            if (entry->bound == EXACT_BND) {
//                if (!entry->hashMove.NAM()) copyPV(ctx, entry->hashMove, NONE_MOVE);
                ctx.decrementLevel();
                return {entry->score, entry->hashMove};
            }

            if (entry->bound & LOWER_BND) alpha = std::max(alpha, entry->score);
            else beta = std::min(beta, entry->score);

            if (alpha >= beta) {
//                if (!entry->hashMove.NAM()) copyPV(ctx, entry->hashMove, NONE_MOVE);
                ctx.decrementLevel();
                return {entry->score, entry->hashMove};
            }
        }
    }

    Score bestScore = even ? -INF : INF;
    int boundType = 0x00;
    if (!depth) {
        auto score = eval_(state, ctx);
        ttdesc.write(score, EXACT_BND, 0, {});
        ctx.decrementLevel();
        return {score, NONE_MOVE};
    }

    brd::MoveList mvList{};
    if constexpr (PV) {
        if (!ctx.T1[0][ctx.relPly].NAM())
            mvList.push(ctx.T1[0][ctx.relPly]);
        else mvList = movegen(even, state, m_opts.EngineSide);
    }
    else {
        mvList = movegen(even, state, m_opts.EngineSide);
    }

    brd::Move bestMove{};
    for (std::size_t i=0; i<mvList.size(); i++) {
        const auto& move = mvList[i];

        state.registerMove(move);
        auto [score, prevMove] = AlphaBeta<PV>(state, alpha, beta, depth-1, !even, ctx);
        state.undo();

        if (even) {
            if (bestScore < score) bestMove = move;
            bestScore = std::max(bestScore, score);

            if (score > alpha) {
                alpha = score;
                copyPV(ctx, move, prevMove);
            }
        }
        else  {
            if (bestScore > score) bestMove = move;
            bestScore = std::min(bestScore, score);
            if (score < beta) {
                beta = score;
                copyPV(ctx, move, prevMove);
            }
        }

        if (alpha >= beta)
            break;
    }

    if (bestScore <= origAlpha) boundType = UPPER_BND;
    else if (bestScore >= origBeta) boundType = LOWER_BND;
    else boundType = EXACT_BND;

    ttdesc.write(bestScore, boundType, depth, bestMove);
    ctx.decrementLevel();
    return {bestScore, bestMove};
}

template <typename TExecutor>
Score MtdSearch<TExecutor>::eval_(brd::BoardState& state, const detail::SearchContext& ctx) noexcept {
    Score eval;
    if (state.checkmate(m_opts.EngineSide))
        eval = static_cast<Score>(-CHECKMATE_EVAL + ctx.relPly);
    else if (state.checkmate(invert(m_opts.EngineSide)))
        eval = static_cast<Score>(CHECKMATE_EVAL - ctx.relPly);
    else
        eval = m_eval.evaluate(state);

    m_stat.NodesSearched++;
    return eval;
}


template class search::MtdSearch<exec::CallerThreadExecutor>;



} // namespace search
