#ifndef INCLUDE_EVAL_EVALUATOR_H_
#define INCLUDE_EVAL_EVALUATOR_H_
#include "../core/defs.h"

namespace brd { class BoardState; class Board; }
namespace common { struct Options; }
namespace eval {
class Evaluator {
public:
    explicit Evaluator(const common::Options& opts) noexcept;
    Score evaluate(const brd::BoardState&) noexcept;

private:
    const common::Options& m_opts;
};
} // namespace eval

#endif  // INCLUDE_EVAL_EVALUATOR_H_
