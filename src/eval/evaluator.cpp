#include "evaluator.h"
#include "../common/options.h"
#include "../board/board_state.h"

namespace eval {

Evaluator::Evaluator(const common::Options& opts) noexcept
    : m_opts(opts) {}

Score Evaluator::evaluate(const brd::BoardState& state) noexcept {
    Score scoreF = state.nonPawnMaterial(m_opts.EngineSide);
    Score scoreS = state.nonPawnMaterial(invert(m_opts.EngineSide));
    return scoreF - scoreS;
}



} // namespace eval
