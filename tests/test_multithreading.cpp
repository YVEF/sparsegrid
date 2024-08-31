#include "board/movegen.h"
#include "common/stat.h"
#include "core/CallerThreadExecutor.h"
#include "eval/evaluator.h"
#include "search/mtdsearch.h"
#include "search/tm.h"
#include "search/tt.h"
#include "test_utils.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <board/board.h>
#include <board/board_state.h>
#include <dbg/debugger.h>
#include <common/options.h>
#include <future>


struct MTTestFixture {
public:
    MTTestFixture() noexcept : opts({}), stat({}), tm({}), ttable(search::TTable{opts, stat}) {
        movegen::init();
        tm.setTimeout(ULONG_MAX);
    }

    common::Options opts;
    common::Stat stat;
    search::TimeManager tm;
    search::TTable ttable;
};

BOOST_AUTO_TEST_SUITE(mtdsearch_test_suite)

/*
 *  . . . k . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . Q K . . .
 */
BOOST_FIXTURE_TEST_CASE(test1, MTTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, B_KING_POS, W_QUEEN_POS});
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(SqNum::sqn_e8, SqNum::sqn_d8));

    opts.MaxDepthPly = 1;
    opts.EngineSide = PColor::W;
    opts.Cores = 12;
    eval::Evaluator evalu{opts};
    search::MtdSearch<exec::CallerThreadExecutor> searcher{opts, stat, tm, ttable, evalu};
    auto res = searcher.pvMove(state);

    BOOST_CHECK_EQUAL(res.pvMove.from, SqNum::sqn_d1);
    BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_d8);
}

BOOST_AUTO_TEST_SUITE_END()