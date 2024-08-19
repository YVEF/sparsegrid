#ifndef INCLUDE_EVAL_EVALUATOR_H_
#define INCLUDE_EVAL_EVALUATOR_H_
#include "../core/defs.h"
#include <memory>

#define INF 16639
#define CHECKMATE_EVAL 8447
#define MIN_CHECKMATE_EVAL 8410
#define SCORE_SCALE_FACTOR 8100


namespace brd { class BoardState; class Board; }
namespace common { struct Options; }
namespace eval {


class Evaluator {
public:
    virtual Score evaluate(const brd::BoardState&) noexcept = 0;
};

class MaterialEvaluator : public Evaluator {
public:
    explicit MaterialEvaluator(const common::Options& opts) noexcept
    : m_opts(opts) {}

    Score evaluate(const brd::BoardState&) noexcept override;

private:
    const common::Options& m_opts;
};


class SGNN;
class NNEvaluator : public Evaluator {
public:
    explicit NNEvaluator(const common::Options& opts);
    Score evaluate(const brd::BoardState&) noexcept;
    ~NNEvaluator();

private:
    const common::Options&  m_opts;
    std::unique_ptr<SGNN>   m_model;
};
} // namespace eval

#endif  // INCLUDE_EVAL_EVALUATOR_H_
