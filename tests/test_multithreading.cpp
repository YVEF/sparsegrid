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
#include <core/ThreadPoolExecutor.h>


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

BOOST_AUTO_TEST_SUITE(multithread_search_test_suite)

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
    opts.Cores = 2;
    eval::Evaluator evalu{opts};
    search::MtdSearch<exec::ThreadPoolExecutor> searcher{opts, stat, tm, ttable, evalu};
    auto res = searcher.pvMove(state);

    BOOST_CHECK_EQUAL(res.pvMove.from, SqNum::sqn_d1);
    BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_d8);
}


/*
 *  . . . . k . . .
 *  . . . . p p p p
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . Q . K . . .
 */
BOOST_FIXTURE_TEST_CASE(test2, MTTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_QUEEN_POS, B_KING_POS, W_KING_POS,
                                  B_PAWN_5_POS, B_PAWN_6_POS, B_PAWN_7_POS, B_PAWN_8_POS});

    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(W_QUEEN_POS, W_QUEEN_POS-1));

    opts.MaxDepthPly = 3;
    opts.Cores = 12;
    opts.EngineSide = PColor::W;
    eval::Evaluator evalu{opts};
    search::MtdSearch<exec::ThreadPoolExecutor> searcher{opts, stat, tm, ttable, evalu};
    auto res = searcher.pvMove(state);

    BOOST_CHECK_EQUAL(res.pvMove.from, SqNum::sqn_c1);
    BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_c8);
}

/*
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . Q . . . .
 *  . . . . . . . .
 *  . . p . . K . .
 *  . . k . . . . .
 */
BOOST_FIXTURE_TEST_CASE(test3, MTTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, { W_QUEEN_POS, W_KING_POS, B_KING_POS, B_PAWN_5_POS });
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(W_QUEEN_POS, SqNum::sqn_d4));
    state.registerMove(brd::mkMove(B_KING_POS, SqNum::sqn_c1));
    state.registerMove(brd::mkMove(W_KING_POS, SqNum::sqn_f2));
    state.registerMove(brd::mkMove(B_PAWN_5_POS, SqNum::sqn_c2));

    opts.MaxDepthPly = 5;
    opts.Cores = 2;
    opts.EngineSide = PColor::W;
    eval::Evaluator evalu{opts};
    search::MtdSearch<exec::ThreadPoolExecutor> searcher{opts, stat, tm, ttable, evalu};
    auto res = searcher.pvMove(state);

    BOOST_CHECK_EQUAL(res.pvMove.from, SqNum::sqn_d4);
    BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_b4);

    BOOST_CHECK_EQUAL(res.ponder.from, SqNum::sqn_c1);
    BOOST_CHECK_EQUAL(res.ponder.to, SqNum::sqn_d1);

    state.registerMove(res.pvMove);
    state.registerMove(res.ponder);
    res = searcher.pvMove(state);

    BOOST_CHECK_EQUAL(res.pvMove.from, SqNum::sqn_b4);
    BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_e1);
}

/*
 *  r . . . r . . k
 *  . . . . N p . .
 *  . . . . . . . .
 *  . . . . . . . n
 *  . . . R . . . .
 *  . . . . . . . P
 *  . B B . . P P .
 *  . . . R . . K .
 */
BOOST_FIXTURE_TEST_CASE(test4, MTTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {});
    board.put(PKind::pR, PColor::B, SqNum::sqn_a8);
    board.put(PKind::pR, PColor::B, SqNum::sqn_e8);
    board.put(PKind::pK, PColor::B, SqNum::sqn_h8);
    board.put(PKind::pN, PColor::W, SqNum::sqn_e7);
    board.put(PKind::pP, PColor::B, SqNum::sqn_f7);
    board.put(PKind::pN, PColor::B, SqNum::sqn_h5);
    board.put(PKind::pR, PColor::W, SqNum::sqn_d4);
    board.put(PKind::pP, PColor::W, SqNum::sqn_h3);
    board.put(PKind::pB, PColor::W, SqNum::sqn_b2);
    board.put(PKind::pB, PColor::W, SqNum::sqn_c2);
    board.put(PKind::pP, PColor::W, SqNum::sqn_f2);
    board.put(PKind::pP, PColor::W, SqNum::sqn_g2);
    board.put(PKind::pR, PColor::W, SqNum::sqn_d1);
    board.put(PKind::pR, PColor::W, SqNum::sqn_g1);

    brd::BoardState state(std::move(board));

    opts.MaxDepthPly = 5;
    opts.Cores = 12;
    opts.EngineSide = PColor::W;
    eval::Evaluator evalu{opts};
    search::MtdSearch<exec::ThreadPoolExecutor> searcher{opts, stat, tm, ttable, evalu};
    auto res = searcher.pvMove(state);

    BOOST_REQUIRE_EQUAL(res.pvMove.from, SqNum::sqn_d4);
    BOOST_REQUIRE_EQUAL(res.pvMove.to, SqNum::sqn_h4);

    state.registerMove(res.pvMove);
    state.registerMove(brd::mkMove(SqNum::sqn_f7, SqNum::sqn_f6));

    res = searcher.pvMove(state);
    BOOST_REQUIRE_EQUAL(res.pvMove.from, SqNum::sqn_b2);
    BOOST_REQUIRE_EQUAL(res.pvMove.to, SqNum::sqn_f6);
}


/*
 *  k . . . . . . .
 *  . . . . p . . .
 *  . R . . . . . .
 *  . . Q . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . K . . .
 */
BOOST_FIXTURE_TEST_CASE(test5, MTTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, { W_QUEEN_POS, B_KING_POS, W_KING_POS, W_ROOK_1_POS, B_PAWN_5_POS });
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(B_KING_POS, SqNum::sqn_a8));
    state.registerMove(brd::mkMove(W_ROOK_1_POS, SqNum::sqn_b6));
    state.registerMove(brd::mkMove(W_QUEEN_POS, SqNum::sqn_c5));

    opts.MaxDepthPly = 100;
    opts.Cores = 100;
    opts.EngineSide = PColor::W;
    eval::Evaluator evalu{opts};
    search::MtdSearch<exec::ThreadPoolExecutor> searcher{opts, stat, tm, ttable, evalu};

    auto result = std::async(std::launch::async, std::mem_fn(&decltype(searcher)::pvMove), &searcher, std::ref(state));
    using namespace std::chrono_literals;
    std::future_status status = result.wait_for(5s);
    BOOST_REQUIRE(status == std::future_status::ready);
    auto res = result.get();

    BOOST_REQUIRE_EQUAL(res.pvMove.from, SqNum::sqn_c5);
    BOOST_REQUIRE(res.pvMove.to == SqNum::sqn_a5 || res.pvMove.to == SqNum::sqn_c8
                  || res.pvMove.to == SqNum::sqn_e7 || res.pvMove.to == SqNum::sqn_a3);
}



BOOST_AUTO_TEST_SUITE_END()