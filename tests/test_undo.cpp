#include "board/movegen.h"
#include "test_utils.h"
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <board/board.h>
#include <board/board_state.h>
#include <dbg/debugger.h>

struct UndoTestFixture {
public:
    UndoTestFixture() noexcept {
        movegen::init();
    }

};


BOOST_AUTO_TEST_SUITE(undo_test_suite)

BOOST_FIXTURE_TEST_CASE(test_undo_1, UndoTestFixture) {
    brd::Board board{};
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4));

    BOOST_CHECK_EQUAL(state.ply(), 1);
    state.undo();

    BOOST_REQUIRE(!state.ply());
    BOOST_REQUIRE(state.getBoard().empty(SqNum::sqn_e4));
    BOOST_REQUIRE(!state.getBoard().empty(SqNum::sqn_e2));
}

BOOST_FIXTURE_TEST_CASE(test_undo_promotion, UndoTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_PAWN_5_POS});
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(sqn_e2, sqn_e4));
    state.registerMove(brd::mkMove(sqn_e4, sqn_e5));
    state.registerMove(brd::mkMove(sqn_e5, sqn_e6));
    state.registerMove(brd::mkMove(sqn_e6, sqn_e7));
    state.registerMove(brd::mkMove(sqn_e7, sqn_e8));


    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_e8), PKind::pQ);
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_e7), PKind::None);

    state.undo();
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_e8), PKind::None);
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_e7), PKind::pP);
}


BOOST_FIXTURE_TEST_CASE(test_undo_enpassant, UndoTestFixture) {
    brd::BoardState state(brd::Board{});

    // add preconditions
    state.registerMove(brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4));
    state.registerMove(brd::mkMove(SqNum::sqn_a7, SqNum::sqn_a6));
    state.registerMove(brd::mkMove(SqNum::sqn_e4, SqNum::sqn_e5));
    state.registerMove(brd::mkMove(SqNum::sqn_d7, SqNum::sqn_d5));
    // enpassant
    
    auto key1 = state.getBoard().key();

    state.registerMove(brd::mkEnpass(SqNum::sqn_e5, SqNum::sqn_d6));
    auto key2 = state.getBoard().key();

    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_d5), PKind::None);
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_d6), PKind::pP);
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_e5), PKind::None);

    state.undo();

    auto key3 = state.getBoard().key();
    BOOST_CHECK(!state.getBoard().empty(SqNum::sqn_d5));
    BOOST_CHECK(state.getBoard().empty(SqNum::sqn_d6));
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_e5), PKind::pP);

    BOOST_CHECK_EQUAL(key1, key3);
    BOOST_CHECK_NE(key1, key2);
}


BOOST_FIXTURE_TEST_CASE(test_undo_castling, UndoTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_ROOK_1_POS, W_KING_POS, W_ROOK_2_POS});
    brd::BoardState state(std::move(board));

    state.registerMove(brd::mkCastling(W_KING_POS, brd::CastlingType::C_SHORT));

    BOOST_CHECK(state.getBoard().empty(W_KING_POS));
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_f1), PKind::pR);
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_g1), PKind::pK);
}


BOOST_FIXTURE_TEST_CASE(test_undo_50_rule, UndoTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, B_KING_POS});
    brd::BoardState state(std::move(board));

    BOOST_REQUIRE(!state.gameover());

    for (int i=0; i<51; i++) {
        auto move = i%2 ? brd::mkMove(SqNum::sqn_e7, SqNum::sqn_e8) : brd::mkMove(SqNum::sqn_e8, SqNum::sqn_e7);
        state.registerMove(move);
    }

    BOOST_CHECK(state.draw());
    BOOST_CHECK_EQUAL(state.ply(), 51);
    state.undo();
    BOOST_CHECK(!state.draw());
    BOOST_CHECK_EQUAL(state.ply(), 50);
}


BOOST_FIXTURE_TEST_CASE(test_undo_7_moves, UndoTestFixture) {
    brd::BoardState state(brd::Board{});

    std::vector<brd::Move> moves = {
        brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4),
        brd::mkMove(SqNum::sqn_c7, SqNum::sqn_c5),
        brd::mkMove(SqNum::sqn_f2, SqNum::sqn_f3),
        brd::mkMove(SqNum::sqn_d7, SqNum::sqn_d5),
        brd::mkMove(SqNum::sqn_e4, SqNum::sqn_d5),
        brd::mkMove(SqNum::sqn_g8, SqNum::sqn_f6),
        brd::mkMove(SqNum::sqn_f1, SqNum::sqn_b5)
    };

    for (auto&& m : moves)
        state.registerMove(m);

    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_b5), PKind::pB);
    BOOST_CHECK_EQUAL(state.getBoard().getColor(Square::sq_b5), PColor::W);

    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_c5), PKind::pP);
    BOOST_CHECK_EQUAL(state.getBoard().getColor(Square::sq_c5), PColor::B);

    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_d5), PKind::pP);
    BOOST_CHECK_EQUAL(state.getBoard().getColor(Square::sq_d5), PColor::W);

    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_e5), PKind::None);

    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_f6), PKind::pN);
    BOOST_CHECK_EQUAL(state.getBoard().getColor(Square::sq_f6), PColor::B);

    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_f3), PKind::pP);
    BOOST_CHECK_EQUAL(state.getBoard().getColor(Square::sq_f3), PColor::W);

    BOOST_CHECK_EQUAL(state.ply(), moves.size());

    for(unsigned i=0; i<moves.size(); i++)
        state.undo();

    BOOST_CHECK(!state.ply());
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_b5), PKind::None);
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_c5), PKind::None);
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_d5), PKind::None);
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_e5), PKind::None);
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_f6), PKind::None);
    BOOST_CHECK_EQUAL(state.getBoard().getKind(Square::sq_f3), PKind::None);
}



BOOST_FIXTURE_TEST_CASE(test_regression_1, UndoTestFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, B_KING_POS, B_PAWN_4_POS, W_QUEEN_POS});
    brd::BoardState state(std::move(board));

    BOOST_REQUIRE_EQUAL(state.getBoard().getColor(1ull << B_PAWN_4_POS), PColor::B);
    state.registerMove(brd::mkMove(W_QUEEN_POS, B_PAWN_4_POS));
    BOOST_REQUIRE_EQUAL(state.getBoard().getColor(1ull << B_PAWN_4_POS), PColor::W);
    state.undo();
    BOOST_REQUIRE_EQUAL(state.getBoard().getColor(1ull << B_PAWN_4_POS), PColor::B);
}



BOOST_AUTO_TEST_SUITE_END()
