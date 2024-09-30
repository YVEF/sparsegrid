#include "mtdsearch.h"
#include "../board/board_state.h"
#include "tm.h"
#include "../common/options.h"
#include "tt.h"
#include "../dbg/sg_assert.h"
#include "../core/CallerThreadExecutor.h"
#include "../eval/evaluator.h"
#include "../common/stat.h"
#include "../core/ThreadPoolExecutor.h"
#include <future>


namespace search {
namespace detail {
struct SearchContext {
    int relPly = -1;
    constexpr static unsigned scMaxPly = 64;
//    unsigned TDepth = 0;
    brd::Move T1[scMaxPly][scMaxPly];
    bool pvWasFound[scMaxPly];
    Score interRes = 0;
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
: m_opts(opts), m_stat(stat), m_ttable(ttable), m_tm(tm), m_eval(eval), m_executor(m_opts) {}


static inline auto getCtxs(const common::Options& opts) {
    std::vector<detail::SearchContext> ctxs(opts.Cores);
    for (auto& c : ctxs)
        c = detail::SearchContext{};
    return ctxs;
}


// todo: fix PVLine
template <typename TExecutor>
search::str::Report MtdSearch<TExecutor>::pvMove(brd::BoardState& state) noexcept {
    m_ttable.incrementAge();

    detail::SearchContext ctx{};
    Score f1 = 0, f2 = 0;
    unsigned depth = 1;
    auto ctxs = getCtxs(m_opts);
    std::vector<brd::BoardState> forkStates(m_opts.Cores, state);

    for (; depth <= m_opts.MaxDepthPly && f1 < MIN_CHECKMATE_EVAL && f1 > -MIN_CHECKMATE_EVAL && !m_tm.timeout(); depth++) {
        if (depth % 2)
            f1 = MTDF_(state, f1, depth, ctx);
        else
            f2 = MTDF_(state, f2, depth, ctx);
    }

    search::str::Report report{};
    report.pvMove = ctx.T1[0][0];
    report.ponder = ctx.T1[0][1];
    std::cout << "pon:" << report.ponder << std::endl;
    SG_ASSERT(!report.pvMove.NAM());

    m_stat.resetSingleSearch();
    return report;
}


template <typename TExecutor>
Score MtdSearch<TExecutor>::MTDF_(brd::BoardState& state, int16_t f, unsigned depth, detail::SearchContext& ctx) noexcept {
//    brd::Move bestMove{};
    int16_t lowerBound = -INF, upperBound = INF, beta = 0;
    while (lowerBound < upperBound && !m_tm.timeout()) {
        beta = (f == lowerBound) ? f+1 : f;
        auto [l, p] = AlphaBeta<false>(state, beta-1, beta, depth, true, ctx);
        f = l;
        if (f < beta) upperBound = f;
        else lowerBound = f;
    }
    return f;
}

static Score checkmateScore(const brd::BoardState& state, PColor engineColor, unsigned relPly) noexcept {
    return state.checkmate(engineColor) ?
        static_cast<Score>(-CHECKMATE_EVAL + relPly)
        : static_cast<Score>(CHECKMATE_EVAL - relPly);
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
        bool even, detail::SearchContext& ctx, bool mainThread) noexcept {

    if (state.gameover()) {
        if (state.draw()) return {0x00, NONE_MOVE};
        return {checkmateScore(state, m_opts.EngineSide, ctx.relPly), NONE_MOVE};
    }

    ctx.incrementLevel();
    auto origAlpha = alpha;
    auto origBeta = beta;

    TTDescriptor ttdesc = m_ttable.probe(state.getBoard().key());
    if (ttdesc.hit()) {
        auto entry = ttdesc.entry();
        if (entry->horizon > depth) {
            if (entry->bound == EXACT_BND) {
                ctx.decrementLevel();
                return {entry->score, entry->hashMove};
            }

            if (entry->bound & LOWER_BND) alpha = std::max(alpha, entry->score);
            else beta = std::min(beta, entry->score);

            if (alpha >= beta) {
                ctx.decrementLevel();
                return {entry->score, entry->hashMove};
            }
        }
    }

    Score bestScore = even ? -INF : INF;
    int boundType = 0x00;
    if (!depth) {
        auto score = eval_(state, ctx.relPly);
        ttdesc.write(score, EXACT_BND, 0, {});
        ctx.decrementLevel();
        return {score, NONE_MOVE};
    }

    brd::MoveList mvList;
    if constexpr (PV) {
        if (!ctx.T1[0][ctx.relPly].NAM())
            mvList.push(ctx.T1[0][ctx.relPly]);
        else mvList = movegen(even, state, m_opts.EngineSide);
    }
    else {
        mvList = movegen(even, state, m_opts.EngineSide);
    }

using spawn_t = std::optional<std::future<std::pair<Score, brd::Move>>>;
#define SPAWN_COND(mt, ii, d) ((ii) < mvList.size()-1 && m_executor.capacity() && (d) >= 3)

    brd::Move bestMove{};

    for (std::size_t i=0; i<mvList.size(); i++) {
        auto move = mvList[i];

        Score score{}; brd::Move prevMove{}; brd::Move spMove{};
        spawn_t spawnFuture;

        if (SPAWN_COND(mainThread, i, depth)) {
            spMove = mvList[i+1];
            spawnFuture = m_executor.try_send(
                [this, &spMove,
                    copy_state = state,
                    alpha, beta, depth, even, ctx]
                    () mutable {
                    copy_state.registerMove(spMove);
                    auto res = AlphaBeta<PV>(copy_state, alpha, beta, depth-1, !even, ctx, false);
                    // skip undo because of copied state
                    return res;
                });
        }

        state.registerMove(move);
        auto [k1, k2] = AlphaBeta<PV>(state, alpha, beta, depth-1, !even, ctx, mainThread);
        score = k1, prevMove = k2;
        state.undo();

        if (spawnFuture.has_value()) {
            SG_ASSERT(!spMove.NAM());

            i++;
            auto [spScore, spMovePrev] = spawnFuture.value().get();
            if ((even && spScore > score) || (!even && spScore < score)) {
                score = spScore;
                prevMove = spMovePrev;
                move = spMove;
            }
        }

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
Score MtdSearch<TExecutor>::eval_(brd::BoardState& state, unsigned relPly) noexcept {
    Score eval;
    if (state.checkmate(m_opts.EngineSide))
        eval = static_cast<Score>(-CHECKMATE_EVAL + relPly);
    else if (state.checkmate(invert(m_opts.EngineSide)))
        eval = static_cast<Score>(CHECKMATE_EVAL - relPly);
    else
        eval = m_eval.evaluate(state);

    m_stat.NodesSearched++;
    return eval;
}

template class search::MtdSearch<exec::CallerThreadExecutor>;
template class search::MtdSearch<exec::ThreadPoolExecutor>;
} // namespace search
