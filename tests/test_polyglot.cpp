#include "test_utils.h"
#include <board/board_state.h>
#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <uci/fen.h>
#include <adapters/polyglot.h>
#include <common/options.h>


BOOST_AUTO_TEST_SUITE(polyglot_test_suite)

BOOST_AUTO_TEST_CASE(test_key_generation) {
    brd::BoardState state(brd::Board{});

    uci::Fen fen;
    std::string_view fen_inputs[] = {
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        "rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1",
        "rnbqkbnr/ppp1pppp/8/3p4/4P3/8/PPPP1PPP/RNBQKBNR w KQkq d6 0 2",
        "rnbqkbnr/ppp1pppp/8/3pP3/8/8/PPPP1PPP/RNBQKBNR b KQkq - 0 2",
        "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3",
        "rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR b kq - 0 3",
        "rnbq1bnr/ppp1pkpp/8/3pPp2/8/8/PPPPKPPP/RNBQ1BNR w - - 0 4",
        "rnbqkbnr/p1pppppp/8/8/PpP4P/8/1P1PPPP1/RNBQKBNR b KQkq c3 0 3",
        "rnbqkbnr/p1pppppp/8/8/P6P/R1p5/1P1PPPP1/1NBQKBNR b Kkq - 0 4"
    };

    std::vector<std::vector<brd::Move>> from_startpos_moves = {
        {}, // start pos
        {brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4)},
        {brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4), brd::mkMove(SqNum::sqn_d7, SqNum::sqn_d5)},
        {brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4), brd::mkMove(SqNum::sqn_d7, SqNum::sqn_d5), brd::mkMove(SqNum::sqn_e4, SqNum::sqn_e5)},
        {brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4), brd::mkMove(SqNum::sqn_d7, SqNum::sqn_d5), brd::mkMove(SqNum::sqn_e4, SqNum::sqn_e5), brd::mkMove(SqNum::sqn_f7, SqNum::sqn_f5)},
        {brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4), brd::mkMove(SqNum::sqn_d7, SqNum::sqn_d5), brd::mkMove(SqNum::sqn_e4, SqNum::sqn_e5), brd::mkMove(SqNum::sqn_f7, SqNum::sqn_f5), brd::mkMove(SqNum::sqn_e1, SqNum::sqn_e2)},
        {brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e4), brd::mkMove(SqNum::sqn_d7, SqNum::sqn_d5), brd::mkMove(SqNum::sqn_e4, SqNum::sqn_e5), brd::mkMove(SqNum::sqn_f7, SqNum::sqn_f5),
            brd::mkMove(SqNum::sqn_e1, SqNum::sqn_e2), brd::mkMove(SqNum::sqn_e8, SqNum::sqn_f7)},

        {brd::mkMove(SqNum::sqn_a2, SqNum::sqn_a4), brd::mkMove(SqNum::sqn_b7, SqNum::sqn_b5), brd::mkMove(SqNum::sqn_h2, SqNum::sqn_h4), brd::mkMove(SqNum::sqn_b5, SqNum::sqn_b4), brd::mkMove(SqNum::sqn_c2, SqNum::sqn_c4)},
        {brd::mkMove(SqNum::sqn_a2, SqNum::sqn_a4), brd::mkMove(SqNum::sqn_b7, SqNum::sqn_b5), brd::mkMove(SqNum::sqn_h2, SqNum::sqn_h4), brd::mkMove(SqNum::sqn_b5, SqNum::sqn_b4),
            brd::mkMove(SqNum::sqn_c2, SqNum::sqn_c4), brd::mkEnpass(SqNum::sqn_b4, SqNum::sqn_c3), brd::mkMove(SqNum::sqn_a1, SqNum::sqn_a3)}
    };

    uint64_t expected_keys[] = {
        0x463b96181691fc9c,
        0x823c9b50fd114196,
        0x0756b94461c50fb0,
        0x662fafb965db29d4,
        0x22a48b5a8e47ff78,
        0x652a607ca3f242c1,
        0x00fdd303c946bdd9,
        0x3c8123ea7b067637,
        0x5c3f9b829b279560
    };

    common::Options opts{};
    opts.EngineSide = PColor::W;


    for(std::size_t i=0; i<std::size(fen_inputs); i++) {
        brd::BoardState c_state(brd::Board{});
        for(auto mv : from_startpos_moves[i])
            c_state.registerMove(mv);

        fen.apply(fen_inputs[i], state);
        auto key = adapters::polyglot::makeKey(state, opts);
        auto c_key = adapters::polyglot::makeKey(c_state, opts);
        std::cout << "1:" << std::hex << key << std::endl;
        std::cout << "2:" << std::hex << c_key << std::endl;
        BOOST_REQUIRE_EQUAL(expected_keys[i], key);
        BOOST_REQUIRE_EQUAL(expected_keys[i], c_key);
    }
}


BOOST_AUTO_TEST_SUITE_END()