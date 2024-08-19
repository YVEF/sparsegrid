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
#include <thread>


struct MtdSearchTestFixture {
public:
    MtdSearchTestFixture() noexcept : opts({}), stat({}), tm({}), ttable(opts, stat) {
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
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . k . . . .
 *  . . . . . . . .
 *  . . . R K . . .
 */
BOOST_FIXTURE_TEST_CASE(test_super_1, MtdSearchTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, B_KING_POS, W_ROOK_1_POS});
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(SqNum::sqn_e8, SqNum::sqn_d8));
    state.registerMove(brd::mkMove(SqNum::sqn_d8, SqNum::sqn_d3));
    state.registerMove(brd::mkMove(W_ROOK_1_POS, SqNum::sqn_d1));

    opts.MaxDepthPly = 1;
    opts.EngineSide = PColor::W;
    eval::MaterialEvaluator evalu{opts};
    search::MtdSearch<exec::CallerThreadExecutor> searcher{opts, stat, tm, ttable, evalu};
    auto res = searcher.pvMove(state);

    BOOST_CHECK_EQUAL(res.pvMove.from, SqNum::sqn_d1);
    BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_d3);
}


/*
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . k . . . . .
 *  . . . . . . . .
 *  . . . R K . . .
 */
BOOST_FIXTURE_TEST_CASE(test_super_2, MtdSearchTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, B_KING_POS, W_ROOK_1_POS});
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(SqNum::sqn_e8, SqNum::sqn_c3));
    state.registerMove(brd::mkMove(W_ROOK_1_POS, SqNum::sqn_d1));

    opts.MaxDepthPly = 3;
    opts.EngineSide = PColor::W;
    eval::MaterialEvaluator evalu{opts};
    search::MtdSearch<exec::CallerThreadExecutor> searcher{opts, stat, tm, ttable, evalu};
    auto res = searcher.pvMove(state);

    state.registerMove(res.pvMove);
    BOOST_CHECK(!state.gameover());
}


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
BOOST_FIXTURE_TEST_CASE(test_find_queen_attack_1, MtdSearchTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, B_KING_POS, W_QUEEN_POS});
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(SqNum::sqn_e8, SqNum::sqn_d8));

    opts.MaxDepthPly = 1;
    opts.EngineSide = PColor::W;
    eval::MaterialEvaluator evalu{opts};
    search::MtdSearch<exec::CallerThreadExecutor> searcher{opts, stat, tm, ttable, evalu};
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
BOOST_FIXTURE_TEST_CASE(test_find_best_queen_to_pawn_attack_depth_3, MtdSearchTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_QUEEN_POS, B_KING_POS, W_KING_POS,
         B_PAWN_5_POS, B_PAWN_6_POS, B_PAWN_7_POS, B_PAWN_8_POS});

    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(W_QUEEN_POS, W_QUEEN_POS-1));

    opts.MaxDepthPly = 3;
    opts.EngineSide = PColor::W;
    eval::MaterialEvaluator evalu{opts};
    search::MtdSearch<exec::CallerThreadExecutor> searcher{opts, stat, tm, ttable, evalu};
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
BOOST_FIXTURE_TEST_CASE(test_w_endgame_1_steps_2_depth_5, MtdSearchTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, { W_QUEEN_POS, W_KING_POS, B_KING_POS, B_PAWN_5_POS });
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(W_QUEEN_POS, SqNum::sqn_d4));
    state.registerMove(brd::mkMove(B_KING_POS, SqNum::sqn_c1));
    state.registerMove(brd::mkMove(W_KING_POS, SqNum::sqn_f2));
    state.registerMove(brd::mkMove(B_PAWN_5_POS, SqNum::sqn_c2));

    opts.MaxDepthPly = 5;
    opts.EngineSide = PColor::W;
    eval::MaterialEvaluator evalu{opts};
    search::MtdSearch<exec::CallerThreadExecutor> searcher{opts, stat, tm, ttable, evalu};
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
 *  . . . . . . . .
 *  . . . . . . . P
 *  k . . K . . . .
 *  . . . . . R . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 */
BOOST_FIXTURE_TEST_CASE(test_w_endgame_3_steps_2_depth_5, MtdSearchTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, { W_KING_POS, B_KING_POS, W_ROOK_2_POS, W_PAWN_8_POS });
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(W_KING_POS, SqNum::sqn_d6));
    state.registerMove(brd::mkMove(B_KING_POS, SqNum::sqn_a6));
    state.registerMove(brd::mkMove(W_ROOK_2_POS, SqNum::sqn_f5));
    state.registerMove(brd::mkMove(W_PAWN_8_POS, SqNum::sqn_h7));

    opts.MaxDepthPly = 5;
    opts.EngineSide = PColor::W;
    eval::MaterialEvaluator evalu{opts};
    search::MtdSearch<exec::CallerThreadExecutor> searcher{opts, stat, tm, ttable, evalu};
    auto res = searcher.pvMove(state);

    if (res.pvMove.from == SqNum::sqn_d6)
        BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_c7);
    else if (res.pvMove.from == SqNum::sqn_h7)
        BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_h8);
    else BOOST_ERROR("Expected move was not found");


    state.registerMove(res.pvMove);
    state.registerMove(brd::mkMove(SqNum::sqn_a6, SqNum::sqn_a7));
    res = searcher.pvMove(state);

    if (res.pvMove.from == SqNum::sqn_f5)
        BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_a5);
    else if (res.pvMove.from == SqNum::sqn_h7)
        BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_h8);
    else BOOST_ERROR("Expected move was not found");
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
BOOST_FIXTURE_TEST_CASE(test_w_endgame_4_steps_2_depth_5, MtdSearchTestFixture) {
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
    opts.EngineSide = PColor::W;
    eval::MaterialEvaluator evalu{opts};
    search::MtdSearch<exec::CallerThreadExecutor> searcher{opts, stat, tm, ttable, evalu};
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
BOOST_FIXTURE_TEST_CASE(test_stop_searching_if_the_best_checkmate_has_been_found, MtdSearchTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, { W_QUEEN_POS, B_KING_POS, W_KING_POS, W_ROOK_1_POS, B_PAWN_5_POS });
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(B_KING_POS, SqNum::sqn_a8));
    state.registerMove(brd::mkMove(W_ROOK_1_POS, SqNum::sqn_b6));
    state.registerMove(brd::mkMove(W_QUEEN_POS, SqNum::sqn_c5));

    opts.MaxDepthPly = 100;
    opts.EngineSide = PColor::W;
    eval::MaterialEvaluator evalu{opts};
    search::MtdSearch<exec::CallerThreadExecutor> searcher{opts, stat, tm, ttable, evalu};

    auto result = std::async(std::launch::async, std::mem_fn(&decltype(searcher)::pvMove), &searcher, std::ref(state));
    using namespace std::chrono_literals;
    std::future_status status = result.wait_for(5s);
    BOOST_REQUIRE(status == std::future_status::ready);
    auto res = result.get();

    BOOST_REQUIRE_EQUAL(res.pvMove.from, SqNum::sqn_c5);
    BOOST_REQUIRE(res.pvMove.to == SqNum::sqn_a5 || res.pvMove.to == SqNum::sqn_c8 
            || res.pvMove.to == SqNum::sqn_e7 || res.pvMove.to == SqNum::sqn_a3);
}




/*
*  . . . k . . . .
*  . . . . . . . .
*  . . . . . . . .
*  . . . . . . q .
*  . . . . . . . .
*  . . . . . B . .
*  . . . . P . . p
*  . . . . K . . .
*/
BOOST_FIXTURE_TEST_CASE(test_b_endgame_6_steps_2_depth_5, MtdSearchTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, B_KING_POS, B_PAWN_8_POS, W_PAWN_5_POS, W_BISHOP_1_POS, B_QUEEN_POS});
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(B_QUEEN_POS, SqNum::sqn_g5));
    state.registerMove(brd::mkMove(B_KING_POS, B_KING_POS-1));
    state.registerMove(brd::mkMove(W_BISHOP_1_POS, SqNum::sqn_f3));
    state.registerMove(brd::mkMove(B_PAWN_8_POS, SqNum::sqn_h2));

    opts.MaxDepthPly = 5;
    opts.EngineSide = PColor::B;
    eval::MaterialEvaluator evalu{opts};
    search::MtdSearch<exec::CallerThreadExecutor> searcher{opts, stat, tm, ttable, evalu};
    auto res = searcher.pvMove(state);

    BOOST_CHECK_EQUAL(res.pvMove.from, SqNum::sqn_g5);
    BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_c1);
    BOOST_CHECK_EQUAL(res.ponder.from, W_KING_POS);
    BOOST_CHECK_EQUAL(res.ponder.to, SqNum::sqn_f2);

    state.registerMove(res.pvMove);
    state.registerMove(res.ponder);

    res = searcher.pvMove(state);

    BOOST_CHECK_EQUAL(res.pvMove.from, SqNum::sqn_c1);
    BOOST_CHECK_EQUAL(res.pvMove.to, SqNum::sqn_g1);
}

// === MULTITHREADING PART


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
BOOST_FIXTURE_TEST_CASE(test_search_cores_2_1, MtdSearchTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, B_KING_POS, W_QUEEN_POS});
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(SqNum::sqn_e8, SqNum::sqn_d8));

    opts.MaxDepthPly = 1;
    opts.EngineSide = PColor::W;
    opts.Cores = 2;
    eval::MaterialEvaluator evalu{opts};
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
BOOST_FIXTURE_TEST_CASE(test_search_cores_12_2, MtdSearchTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_QUEEN_POS, B_KING_POS, W_KING_POS,
                                  B_PAWN_5_POS, B_PAWN_6_POS, B_PAWN_7_POS, B_PAWN_8_POS});

    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(W_QUEEN_POS, W_QUEEN_POS-1));

    opts.MaxDepthPly = 3;
    opts.Cores = 12;
    opts.EngineSide = PColor::W;
    eval::MaterialEvaluator evalu{opts};
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
BOOST_FIXTURE_TEST_CASE(test_search_cores_2_3, MtdSearchTestFixture) {
    std::cout << "main tid:" << std::this_thread::get_id() << std::endl;
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
    eval::MaterialEvaluator evalu{opts};
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
BOOST_FIXTURE_TEST_CASE(test_search_cores_12_4, MtdSearchTestFixture) {
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
    eval::MaterialEvaluator evalu{opts};
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
BOOST_FIXTURE_TEST_CASE(test_search_cores_100_5, MtdSearchTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, { W_QUEEN_POS, B_KING_POS, W_KING_POS, W_ROOK_1_POS, B_PAWN_5_POS });
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(B_KING_POS, SqNum::sqn_a8));
    state.registerMove(brd::mkMove(W_ROOK_1_POS, SqNum::sqn_b6));
    state.registerMove(brd::mkMove(W_QUEEN_POS, SqNum::sqn_c5));

    opts.MaxDepthPly = 100;
    opts.Cores = 100;
    opts.EngineSide = PColor::W;
    eval::MaterialEvaluator evalu{opts};
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

// ======================


BOOST_AUTO_TEST_SUITE_END()
