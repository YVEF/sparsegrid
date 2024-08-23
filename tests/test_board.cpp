#include "test_utils.h"
#include <bitset>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <unordered_set>
#include <board/board.h>


BOOST_AUTO_TEST_SUITE(board_test_suite)



BOOST_AUTO_TEST_CASE(test_initial_position) {
    brd::Board board{};
    auto sq_w_q = board.getPieceSqMask<PColor::W, PKind::pQ>();
    auto sq_w_r = board.getPieceSqMask<PColor::W, PKind::pR>();
    auto sq_w_b = board.getPieceSqMask<PColor::W, PKind::pB>();
    auto sq_w_p = board.getPieceSqMask<PColor::W, PKind::pP>();
    auto sq_w_k = board.getPieceSqMask<PColor::W, PKind::pK>();
    auto sq_w_n = board.getPieceSqMask<PColor::W, PKind::pN>();

    BOOST_REQUIRE_EQUAL(sq_w_q, 0x08);
    BOOST_REQUIRE_EQUAL(sq_w_r, 0x81);
    BOOST_REQUIRE_EQUAL(sq_w_b, 0x24);
    BOOST_REQUIRE_EQUAL(sq_w_p, 0xFF00);
    BOOST_REQUIRE_EQUAL(sq_w_k, 0x10);
    BOOST_REQUIRE_EQUAL(sq_w_n, 0x42);
}


BOOST_AUTO_TEST_CASE(test_moves) {
    brd::Board board{};
    auto sq_b_p = board.getPieceSqMask<PColor::B, PKind::pP>();
    board.slideTo(48, 40);
    auto sq_b_p_2 = board.getPieceSqMask<PColor::B, PKind::pP>();
    board.slideTo(40, 32);
    auto sq_b_p_3 = board.getPieceSqMask<PColor::B, PKind::pP>();

    board.slideTo(52, 44);
    board.slideTo(60, 52);
    auto sq_b_k = board.getPieceSqMask<PColor::B, PKind::pK>();

    BOOST_REQUIRE_EQUAL(sq_b_p, 0xFF000000000000);
    BOOST_REQUIRE_EQUAL(sq_b_p_2, 0xFE010000000000);
    BOOST_REQUIRE_EQUAL(sq_b_p_3, 0xFE000100000000);
    BOOST_REQUIRE_EQUAL(sq_b_k, 0x10000000000000);
}


BOOST_AUTO_TEST_CASE(test_kill) {
    brd::Board board{};
    auto before_p = board.getPieceSqMask<PColor::W, PKind::pP>();
    board.kill(8);
    auto after_p = board.getPieceSqMask<PColor::W, PKind::pP>();
    BOOST_REQUIRE_EQUAL(before_p, 0xFF00);
    BOOST_REQUIRE_EQUAL(after_p, 0xFE00);

    auto before_q = board.getPieceSqMask<PColor::W, PKind::pQ>();
    board.kill(std::countr_zero(before_q));
    auto after_q = board.getPieceSqMask<PColor::W, PKind::pQ>();
    BOOST_REQUIRE_EQUAL(before_q, 0x08);
    BOOST_REQUIRE_EQUAL(after_q, 0x00);
}





BOOST_AUTO_TEST_SUITE_END()
