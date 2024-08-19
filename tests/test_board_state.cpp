#include "board/move.h"
#include "board/movegen.h"
#include "dbg/debugger.h"
#include "test_utils.h"
#include <board/board.h>
#include <board/board_state.h>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>

#include <dbg/debugger.h>

struct BoardStateFixture {
public:
    BoardStateFixture() noexcept {
        movegen::init();
    }
};


BOOST_AUTO_TEST_SUITE(board_state_test_suite)

BOOST_FIXTURE_TEST_CASE(test_sliding_moves_1, BoardStateFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, 
            {W_ROOK_1_POS, 
            W_BISHOP_1_POS, W_QUEEN_POS,
            W_KING_POS, W_BISHOP_2_POS, 
            B_BISHOP_1_POS, B_ROOK_1_POS});

    brd::BoardState state(std::move(board));
    auto mvList = brd::MoveList{};
    state.movegen(mvList);

    auto brd = state.getBoard();
    brd::MoveMask mmWRook{};
    brd::MoveMask mmQueen{};
    brd::MoveMask mmBRook{};

    while (mvList.size()) {
        auto move = mvList.pop();
        if (move.from == W_ROOK_1_POS) {
            if (brd.empty(move.to)) mmWRook.qmoves |= 1ull << move.to;
            else mmWRook.amoves |= 1ull << move.to;
        }
        else if(move.from == W_QUEEN_POS) {
            if (brd.empty(move.to)) mmQueen.qmoves |= 1ull << move.to;
            else mmQueen.amoves |= 1ull << move.to;
        }
        else if(move.from == B_ROOK_1_POS) {
            if (brd.empty(move.to)) mmBRook.qmoves |= 1ull << move.to;
            else mmBRook.amoves |= 1ull << move.to;
        }
    }

    uint64_t expectedWRook = flushBit(flushBit(NFile::fA, B_ROOK_1_POS), W_ROOK_1_POS) | 0x02;
    BOOST_REQUIRE_EQUAL(mmWRook.qmoves, expectedWRook);
    BOOST_REQUIRE_EQUAL(mmWRook.amoves, 1ULL << B_ROOK_1_POS);

    uint64_t expectedQueen = flushBit(NFile::fD, W_QUEEN_POS);
    expectedQueen = expectedQueen
        | Square::sq_a4 | Square::sq_b3 | Square::sq_c2 
        | Square::sq_e2 | Square::sq_f3 | Square::sq_g4 | Square::sq_h5;

    BOOST_REQUIRE_EQUAL(mmQueen.qmoves, expectedQueen);
    BOOST_REQUIRE_EQUAL(mmQueen.amoves, 0x00);

    uint64_t expectedBRook = flushBit(flushBit(NFile::fA, B_ROOK_1_POS), W_ROOK_1_POS) | (1ULL << (B_ROOK_1_POS+1));
    BOOST_REQUIRE_EQUAL(mmBRook.qmoves, expectedBRook);
    BOOST_REQUIRE_EQUAL(mmBRook.amoves, 1ULL << W_ROOK_1_POS);
}


BOOST_FIXTURE_TEST_CASE(test_nonsliding_moves_1, BoardStateFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, 
        {B_KING_POS, W_KING_POS, W_KNIGHT_1_POS, B_KNIGHT_2_POS, W_PAWN_1_POS});

    brd::BoardState state(std::move(board));
    brd::MoveList mvList{};
    state.movegen(mvList);

    brd::MoveMask mmWPawn1{};
    brd::MoveMask mmWKnight1{};
    brd::MoveMask mmBKnight2{};
    brd::MoveMask mmBKing{};
    brd::MoveMask mmWKing{};


    auto&& brd = state.getBoard();
    while(mvList.size()) {
        auto move = mvList.pop();
        if(move.from == W_PAWN_1_POS) {
            if (brd.empty(move.to)) mmWPawn1.qmoves |= 1ull << move.to;
            else mmWPawn1.amoves |= 1ull << move.to;
        }
        else if(move.from == W_KING_POS) {
            if (brd.empty(move.to)) mmWKing.qmoves |= 1ull << move.to;
            else mmWKing.amoves |= 1ull << move.to;
        }
        else if(move.from == W_KNIGHT_1_POS) {
            if (brd.empty(move.to)) mmWKnight1.qmoves |= 1ull << move.to;
            else mmWKnight1.amoves |= 1ull << move.to;
        }
        else if(move.from == B_KNIGHT_2_POS) {
            if (brd.empty(move.to)) mmBKnight2.qmoves |= 1ull << move.to;
            else mmBKnight2.amoves |= 1ull << move.to;
        }
        else if(move.from == B_KING_POS) {
            if (brd.empty(move.to)) mmBKing.qmoves |= 1ull << move.to;
            else mmBKing.amoves |= 1ull << move.to;
        }
    }

    BOOST_REQUIRE_EQUAL(mmWPawn1.qmoves, (1ull << (W_PAWN_1_POS+8)) | (1ull << (W_PAWN_1_POS+16)));
    BOOST_REQUIRE_EQUAL(mmWPawn1.amoves, 0x00);


    uint64_t expectedWKing = (1ull << (W_KING_POS - 1))
        | (1ull << (W_KING_POS + 1))
        | (1ull << (W_KING_POS + 8))
        | (1ull << (W_KING_POS + 7))
        | (1ull << (W_KING_POS + 9));

    BOOST_REQUIRE_EQUAL(mmWKing.qmoves, expectedWKing);
    BOOST_REQUIRE_EQUAL(mmWKing.amoves, 0x00);


    uint64_t expectedWKnight1 = (1ull << 16)
        | (1ull << 18)
        | (1ull << 11);

    BOOST_REQUIRE_EQUAL(mmWKnight1.qmoves, expectedWKnight1);
    BOOST_REQUIRE_EQUAL(mmWKnight1.amoves, 0x00);

    uint64_t expectedBKnight2 = (1ull << 47)
        | (1ull << 45)
        | (1ull << 52);

    BOOST_REQUIRE_EQUAL(mmBKnight2.qmoves, expectedBKnight2);
    BOOST_REQUIRE_EQUAL(mmBKnight2.amoves, 0x00);


    uint64_t expectedBKing = (1ull << (B_KING_POS - 1))
        | (1ull << (B_KING_POS + 1))
        | (1ull << (B_KING_POS - 8))
        | (1ull << (B_KING_POS - 7))
        | (1ull << (B_KING_POS - 9));

    BOOST_REQUIRE_EQUAL(mmBKing.qmoves, expectedBKing);
    BOOST_REQUIRE_EQUAL(mmBKing.amoves, 0x00);

}

/*
 * . . . . k . . r
 * . . . . . . . .
 * . . . . . . . .
 * . . . . . . . .
 * . . . . . . . .
 * . . . . . . . .
 * . . . . . . . .
 * R . . . K . . .
 */
BOOST_FIXTURE_TEST_CASE(test_castling_move, BoardStateFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, 
        {B_KING_POS, W_KING_POS, W_ROOK_1_POS, B_ROOK_2_POS});

    brd::BoardState state(std::move(board));
    auto mvList = brd::MoveList{};
    state.movegen(mvList);

    BOOST_CHECK_EQUAL(mvList.size(), 31);

    int whiteCastle = 0, blackCastle = 0;
    while(mvList.size()) {
        auto move = mvList.pop();
        if(move.castling && move.from == W_KING_POS) {
            BOOST_CHECK_EQUAL(move.castling, brd::CastlingType::C_LONG);
            whiteCastle++;
        }
        else if(move.castling && move.from == B_KING_POS) {
            BOOST_CHECK_EQUAL(move.castling, brd::CastlingType::C_SHORT);
            blackCastle++;
        }
    }

    BOOST_CHECK_EQUAL(whiteCastle, 1);
    BOOST_CHECK_EQUAL(blackCastle, 1);
}

BOOST_FIXTURE_TEST_CASE(test_move_registration, BoardStateFixture) {
    brd::Board board{};
    brd::BoardState state(std::move(board));

    state.registerMove(brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4));

    BOOST_REQUIRE(state.getBoard().empty(SqNum::sqn_e2));
    BOOST_REQUIRE(!state.getBoard().empty(SqNum::sqn_e4));
}


BOOST_FIXTURE_TEST_CASE(test_enpassant_move_1, BoardStateFixture) {
    brd::Board board{};
    brd::BoardState state(std::move(board));
    // add preconditions
    state.registerMove(brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4));
    state.registerMove(brd::mkMove(SqNum::sqn_a7, SqNum::sqn_a6));
    state.registerMove(brd::mkMove(SqNum::sqn_e4, SqNum::sqn_e5));
    state.registerMove(brd::mkMove(SqNum::sqn_d7, SqNum::sqn_d5));
    brd::MoveList mvList{};

    state.movegenFor<PColor::W, PKind::pP>(mvList);

    bool enpassFound = false;
    while(mvList.size()) {
        auto move = mvList.pop();
        if(move.isEnpass) {
            enpassFound = true;
            BOOST_REQUIRE_EQUAL(move.to, SqNum::sqn_d6);
        }
    }

    BOOST_REQUIRE(enpassFound);
}


BOOST_FIXTURE_TEST_CASE(test_no_castling_if_king_was_moved, BoardStateFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, W_ROOK_1_POS, W_ROOK_2_POS, B_KING_POS, B_ROOK_1_POS, B_ROOK_2_POS});
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(W_KING_POS, W_KING_POS+1));
    state.registerMove(brd::mkMove(W_KING_POS+1, W_KING_POS));

    brd::MoveList mvList{};
    state.movegenFor<PColor::W, PKind::pK>(mvList);
    state.movegenFor<PColor::B, PKind::pK>(mvList);

    unsigned wc = 0, bc = 0, err = 0;
    while (mvList.size()) {
        auto move = mvList.pop();
        if (move.castling) {
            if (move.from == W_KING_POS) wc++;
            else if (move.from == B_KING_POS) bc++;
            else err++;
        }
    }

    BOOST_REQUIRE(!err);
    BOOST_REQUIRE_EQUAL(wc, 0x00);
    BOOST_REQUIRE_EQUAL(bc, 0x02);
}

BOOST_FIXTURE_TEST_CASE(test_no_castling_if_king_under_check, BoardStateFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, W_ROOK_1_POS, B_QUEEN_POS});
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(B_QUEEN_POS, B_QUEEN_POS+1));
    brd::MoveList mvList{};
    state.movegenFor<PColor::W, PKind::pK>(mvList);

    unsigned c = 0;
    while (mvList.size())
        if (mvList.pop().castling) c++;

    BOOST_REQUIRE(!c);

    state.registerMove(brd::mkMove(B_QUEEN_POS+1, B_QUEEN_POS));
    state.movegenFor<PColor::W, PKind::pK>(mvList);

    while (mvList.size())
        if (mvList.pop().castling) c++;
    
    BOOST_REQUIRE_EQUAL(c, 0x01);
}


BOOST_FIXTURE_TEST_CASE(test_material_increment_update, BoardStateFixture) {
    brd::Board board{};
    brd::BoardState state(std::move(board));

    BOOST_REQUIRE_EQUAL(state.nonPawnMaterial(PColor::W), state.nonPawnMaterial(PColor::B));

    state.registerMove(brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4));
    state.registerMove(brd::mkMove(SqNum::sqn_d7, SqNum::sqn_d5));
    state.registerMove(brd::mkMove(SqNum::sqn_e4, SqNum::sqn_d5));
    state.registerMove(brd::mkMove(SqNum::sqn_d8, SqNum::sqn_d5));
    state.registerMove(brd::mkMove(SqNum::sqn_f1, SqNum::sqn_c4));
    state.registerMove(brd::mkMove(SqNum::sqn_d5, SqNum::sqn_c4));


    BOOST_REQUIRE_GT(state.nonPawnMaterial(PColor::B), state.nonPawnMaterial(PColor::W));
}


BOOST_FIXTURE_TEST_CASE(test_regression_1, BoardStateFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_QUEEN_POS, B_KING_POS, W_KING_POS});
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(W_QUEEN_POS, W_ROOK_1_POS));

    brd::MoveList mvList{};
    state.movegen(mvList);

    bool castling = false;
    while (mvList.size()) if (mvList.pop().castling) castling = true;

    BOOST_REQUIRE(!castling);
}


BOOST_FIXTURE_TEST_CASE(test_regression_2, BoardStateFixture) {
    brd::BoardState state(brd::Board{});
    state.registerMove(brd::mkMove(14, 30));
    state.registerMove(brd::mkMove(62, 45));
    state.registerMove(brd::mkMove(30, 38));
    state.registerMove(brd::mkMove(55, 39));

    brd::MoveList mvList{};
    state.movegenFor<PColor::W>(mvList);

    brd::Move enpassMove;
    while(mvList.size()) {
        auto move = mvList.pop();
        if (move.isEnpass) enpassMove = move;
    }

    BOOST_REQUIRE(!enpassMove.NAM());
    BOOST_REQUIRE_EQUAL(enpassMove.from, 38);
    BOOST_REQUIRE_EQUAL(enpassMove.to, 47);
}


BOOST_FIXTURE_TEST_CASE(test_regression_3, BoardStateFixture) {
    brd::BoardState state(brd::Board{});
    state.registerMove(brd::mkMove(12, 28));
    state.registerMove(brd::mkMove(57, 42));
    state.registerMove(brd::mkMove(6, 23));
    state.registerMove(brd::mkMove(42, 25));
    state.registerMove(brd::mkMove(1, 18));
    state.registerMove(brd::mkMove(25, 10));
    state.registerMove(brd::mkMove(23, 38));
    state.registerMove(brd::mkMove(10, 4));

    brd::MoveList mvList{};
    state.movegenFor<PColor::W>(mvList);

    while(mvList.size()) {
        auto move = mvList.pop();
        BOOST_CHECK(!(move.from == 13 && move.to == 4));
    }
}

/* Test state:
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . k . R . . . .
 *  . . . R K . . .
 */
BOOST_FIXTURE_TEST_CASE(test_regression_4, BoardStateFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, B_KING_POS, W_ROOK_1_POS});
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(W_ROOK_1_POS, SqNum::sqn_d1));
    state.registerMove(brd::mkMove(B_KING_POS, SqNum::sqn_c3));

    auto keyInit = state.getBoard().key();

    state.registerMove(brd::mkMove(SqNum::sqn_d1, SqNum::sqn_d5));
    state.registerMove(brd::mkMove(SqNum::sqn_c3, SqNum::sqn_b2));
    state.registerMove(brd::mkMove(SqNum::sqn_d5, SqNum::sqn_d2));

    auto key2 = state.getBoard().key();

    state.undo();
    state.undo();
    state.undo();

    state.registerMove(brd::mkMove(SqNum::sqn_d1, SqNum::sqn_d2));
    state.registerMove(brd::mkMove(SqNum::sqn_c3, SqNum::sqn_b2));

    auto key3 = state.getBoard().key();
    state.undo();
    state.undo();
    auto keyInit2 = state.getBoard().key();


    BOOST_CHECK_EQUAL(keyInit, keyInit2);
    BOOST_CHECK_NE(key2, key3);
    BOOST_CHECK_NE(keyInit, key2);
    BOOST_CHECK_NE(keyInit, key3);
}


/* Test state:
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . . . . . . .
 *  . . p k . K . .
 *  . . . . Q . . .
 */
BOOST_FIXTURE_TEST_CASE(test_regression_5, BoardStateFixture) {
    brd::Board board{};
    preserveOnlyPositions(board, {W_KING_POS, B_KING_POS, W_QUEEN_POS, B_PAWN_3_POS});
    board.rebuildKey();
    brd::BoardState state(std::move(board));
    state.registerMove(brd::mkMove(W_QUEEN_POS, SqNum::sqn_d4));
    state.registerMove(brd::mkMove(B_KING_POS, SqNum::sqn_c1));
    state.registerMove(brd::mkMove(W_KING_POS, SqNum::sqn_f2));
    state.registerMove(brd::mkMove(B_PAWN_3_POS, SqNum::sqn_c2));

    auto keyInit = state.getBoard().key();
    std::cout << keyInit << std::endl;

    state.registerMove(brd::mkMove(SqNum::sqn_d4, SqNum::sqn_a1)); // w
    state.registerMove(brd::mkMove(SqNum::sqn_c1, SqNum::sqn_d2)); // b
    state.registerMove(brd::mkMove(SqNum::sqn_a1, SqNum::sqn_e1)); // w

    // black to move
    auto key2 = state.getBoard().key();

    std::cout << key2 << std::endl;

    state.undo();
    state.undo();
    state.undo();

    auto keyInit3 = state.getBoard().key();
    std::cout << keyInit3 << std::endl;

    state.registerMove(brd::mkMove(SqNum::sqn_d4, SqNum::sqn_b4)); // w
    state.registerMove(brd::mkMove(SqNum::sqn_c1, SqNum::sqn_d1)); // b
    state.registerMove(brd::mkMove(SqNum::sqn_b4, SqNum::sqn_e1)); // w
    state.registerMove(brd::mkMove(SqNum::sqn_d1, SqNum::sqn_d2)); // b

    // white to move
    auto key3 = state.getBoard().key();
    std::cout << key3 << std::endl;
    state.undo();
    state.undo();
    state.undo();
    state.undo();
    auto keyInit2 = state.getBoard().key();

    std::cout << keyInit2 << std::endl;

    BOOST_CHECK_EQUAL(keyInit, keyInit2);
    BOOST_CHECK_EQUAL(keyInit, keyInit3);
    BOOST_CHECK_NE(key2, key3);
    BOOST_CHECK_NE(keyInit, key2);
    BOOST_CHECK_NE(keyInit, key3);
}


template<std::size_t I>
static void fillBits(double* input, const auto& rawBrd) {
    auto brdPart = std::get<I>(rawBrd);
    constexpr auto start = I*64;
    for (SQ i=0; i<64; i++) {
        if ((1ull << i) & brdPart) input[start+i] = 1.0;
    }
}

void rebuildNN(const brd::BoardState& state, double* data) noexcept {
    std::fill_n(data, brd::nnLayerSize(), 0.0);

    auto rawBrd = state.getBoard().getRawBoard();
    fillBits<0>(data, rawBrd);
    fillBits<1>(data, rawBrd);
    fillBits<2>(data, rawBrd);
    fillBits<3>(data, rawBrd);
    if (getNextPlayerColor(state))
        data[256] = 1.0;
    else
        data[319] = 1.0;
}

static int checkDouble(const double* a, const double* b, std::size_t size) {
    for (std::size_t i=0; i<size; i++) {
        if (a[i] > b[i]) return 1;
        if (a[i] < b[i]) return -1;
    }
    return 0;
}

BOOST_FIXTURE_TEST_CASE(test_run_game_check_consistent_nn_layer, BoardStateFixture) {
    brd::BoardState state(brd::Board{});
    std::vector<brd::Move> game = {
        brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4),
        brd::mkMove(SqNum::sqn_e7, SqNum::sqn_e5),
        brd::mkMove(SqNum::sqn_g1, SqNum::sqn_f3),
        brd::mkMove(SqNum::sqn_f7, SqNum::sqn_f6),
        brd::mkMove(SqNum::sqn_f1, SqNum::sqn_c4),
        brd::mkMove(SqNum::sqn_g8, SqNum::sqn_e7),

    };

    double* refData = new double[brd::nnLayerSize()];
    rebuildNN(state, refData);
    auto r = checkDouble(refData, state.getNNL().data(), brd::nnLayerSize());
    BOOST_REQUIRE(!r);

    for (auto&& mv : game) {
        state.registerMove(mv);
        rebuildNN(state, refData);
        r = checkDouble(refData, state.getNNL().data(), brd::nnLayerSize());
        BOOST_CHECK(!r);
    }

    for (std::size_t i=0; i<game.size(); i++) {
        state.undo();
        rebuildNN(state, refData);
        r = checkDouble(refData, state.getNNL().data(), brd::nnLayerSize());
        BOOST_CHECK(!r);
    }

    delete[] refData;
}





BOOST_AUTO_TEST_SUITE_END()

