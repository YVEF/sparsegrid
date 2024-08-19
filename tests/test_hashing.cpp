#include "board/movegen.h"
#include "common/options.h"
#include "common/stat.h"
#include <board/board.h>
#include <board/board_state.h>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <unordered_set>
#include <dbg/debugger.h>



BOOST_AUTO_TEST_SUITE(hashing_test_suite)
struct HashingTestFixture {
public:
    HashingTestFixture () noexcept {
        movegen::init();
    }

    common::Options opts;
    common::Stat    stat;
};




BOOST_FIXTURE_TEST_CASE(test_same_board_state_from_different_pathes_has_same_stamp, HashingTestFixture) {
    brd::BoardState state(brd::Board{});
    auto key1 = state.getBoard().key();
    state.registerMove(brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e3));
    state.registerMove(brd::mkMove(SqNum::sqn_e7, SqNum::sqn_e5));
    state.registerMove(brd::mkMove(SqNum::sqn_d2, SqNum::sqn_d4));
    state.registerMove(brd::mkMove(SqNum::sqn_a7, SqNum::sqn_a5));
    auto key2 = state.getBoard().key();
    state.undo(); state.undo(); state.undo(); state.undo();

    state.registerMove(brd::mkMove(SqNum::sqn_d2, SqNum::sqn_d4));
    state.registerMove(brd::mkMove(SqNum::sqn_a7, SqNum::sqn_a5));
    state.registerMove(brd::mkMove(SqNum::sqn_e2, SqNum::sqn_e3));
    state.registerMove(brd::mkMove(SqNum::sqn_e7, SqNum::sqn_e5));

    auto key3 = state.getBoard().key();
    state.undo(); state.undo(); state.undo(); state.undo();
    auto key4 = state.getBoard().key();

    BOOST_REQUIRE_EQUAL(key1, key4);
    BOOST_REQUIRE_EQUAL(key2, key3);
    BOOST_REQUIRE_NE(key1, key2);
}


void runRecursive(brd::BoardState& state, bool w, unsigned depth, auto callback) noexcept {
        std::invoke(callback, state);
        if (!depth) return;

        brd::MoveList mvList{};
        if (w) {
            state.movegenFor<PColor::W>(mvList);
        }
        else {
            state.movegenFor<PColor::B>(mvList);
        }
        
        while(mvList.size()) {
            auto move = mvList.pop();
            state.registerMove(move);
            runRecursive(state, !w, depth-1, callback);
            state.undo();
        }
    };


BOOST_FIXTURE_TEST_CASE(test_first_level_unique_keys_equal_amount_of_moves_plus_init_state, HashingTestFixture) {
    brd::BoardState state(brd::Board{});
    std::unordered_set<brd::BrdKey_t> keys{};

    runRecursive(state, true, 3, [&](brd::BoardState& state) {
        keys.emplace(state.getBoard().key());
    });

    BOOST_REQUIRE_EQUAL(keys.size(), 5783);
}

BOOST_AUTO_TEST_SUITE_END()

