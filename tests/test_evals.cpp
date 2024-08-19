#include "test_utils.h"
#include <bitset>
#include <boost/test/unit_test.hpp>
#include <boost/test/unit_test_suite.hpp>
#include <unordered_set>
#include <board/board.h>
#include <eval/evaluator.h>
#include <common/options.h>
#include <board/board_state.h>

BOOST_AUTO_TEST_SUITE(evals_test_suite)


BOOST_AUTO_TEST_CASE(test_local, *boost::unit_test::disabled()) {
    common::Options opts{};
    eval::NNEvaluator evalu(opts);
    brd::BoardState state(brd::Board{});
    brd::MoveList mvList{};
    state.movegenFor<PColor::W>(mvList);
    std::vector<Score> scores{};
    brd::Move bestMove{};
    Score bestScore = -SCORE_SCALE_FACTOR;
    for (auto i=0; i<mvList.size(); i++) {
        state.registerMove(mvList[i]);
        scores.push_back(evalu.evaluate(state));
        state.undo();
        if (bestScore < scores.back()) {
            bestScore = scores.back();
            bestMove = mvList[i];
        }
    }

    for(auto sc : scores) {
        std::cout << sc << std::endl;
    }

    std::cout << "best score:" << bestScore << " best move:" << bestMove << std::endl;
}

BOOST_AUTO_TEST_SUITE_END()