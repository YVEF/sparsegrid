#include "board/movegen.h"
#include <Eigen/Dense>
#include <unsupported/Eigen/CXX11/Tensor>
#include <board/board.h>
#include <board/board_state.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <dbg/debugger.h>
#include <uci/fen.h>


BOOST_AUTO_TEST_SUITE(fen_test_suite)


static void assertFenString(std::string_view input, brd::BoardState& state) {
    std::size_t i=0;
    SQ sq = SqNum::sqn_a8;
    for(; i<input.size() && input[i] != ' '; i++) {
        char c = input[i];
        if(c == '/') {
            sq -= 16;
            continue;
        }
        if(std::isdigit(c)) {
            int d = c - '0';
            while(d-- > 0) BOOST_CHECK(state.getBoard().empty(sq++));
            continue;
        }
        auto [kind, color] = c2p(c);
        auto pkind = state.getBoard().getKind(1ull << sq);
        auto pcol = state.getBoard().getColor(1ull << sq);
        sq++;

        BOOST_CHECK_EQUAL((int)pkind, (int)kind);
        BOOST_CHECK_EQUAL(pcol, color);
    }
}


BOOST_AUTO_TEST_CASE(test_init_position) {
    brd::BoardState state(brd::Board{});
    uci::Fen fen;

    std::string_view input = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    fen.apply(input, state);
    
    // ***** ASSERT
    assertFenString(input, state);
}


BOOST_AUTO_TEST_CASE(test_e2e4_w) {
    brd::BoardState state(brd::Board{});
    uci::Fen fen;
    std::string_view input = "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1";
    fen.apply(input, state);

    // ***** ASSERT
    assertFenString(input, state);
}

BOOST_AUTO_TEST_CASE(test_position_1) {
    brd::BoardState state(brd::Board{});
    uci::Fen fen;
    std::string_view input = "2kr3r/pp4pp/n1p2n2/2b5/P1p1P1P1/2P5/2P1R2P/R3K2q w - - 3 18";
    fen.apply(input, state);

    // ***** ASSERT
    assertFenString(input, state);
}

BOOST_AUTO_TEST_SUITE_END()
