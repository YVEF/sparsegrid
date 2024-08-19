#include "evaluator.h"
#include "../common/options.h"
#include "../board/board_state.h"
#include <Eigen/Dense>
#include <memory>
#include <gzip/decompress.hpp>
#include <fstream>
#include <string>

namespace eval {


class SGNN {
public:
    Eigen::MatrixXd l1w{545, 320};
    Eigen::VectorXd l1b{545};
    Eigen::MatrixXd l2w{170, 545};
    Eigen::VectorXd l2b{170};
    Eigen::MatrixXd outw{1, 170};
    Eigen::VectorXd outb{1};

    [[nodiscard]] double forward(const brd::BoardState::nnLayer_t& input) const {
//        auto nnState = quantize(input);
        Eigen::Map<Eigen::Matrix<double, 1, brd::nnLayerSize(), Eigen::RowMajor>> nnState(const_cast<double*>(input.data()));

//        auto nnState = M;

        auto s1 = nnState * l1w.transpose();
        auto s2 = s1 + l1b.transpose();
        auto s3 = s2.unaryExpr(&SGNN::LeakyReLU1);

        auto s4 = s3 * l2w.transpose();
        auto s5 = s4 + l2b.transpose();
        auto s6 = s5.unaryExpr(&SGNN::LeakyReLU2);

        auto s7 = s6 * outw.transpose();
        auto s8 = s7 + outb.transpose();
        auto s9 = s8.unaryExpr(&SGNN::sigmoid);
        return s9(0, 0);
    }

    static double tanh(double x) {
        return std::tanh(x);
    }

    static double LeakyReLU1(double x) noexcept {
        constexpr double neg_slope = 0.1;
        if (x >= 0.0) return x;
        return neg_slope * x;
    }

    static double LeakyReLU2(double x) noexcept {
        constexpr double neg_slope = 0.3;
        if (x >= 0.0) return x;
        return neg_slope * x;
    }

    static double ReLU6(double x) noexcept {
        return std::min(std::max(0.0, x), 6.0);
    }

    static double sigmoid(double x) noexcept {
        return 1.0/(1.0 + std::exp(-x));
    }
};


template<typename TT, int TS1, int TS2>
void read_csv(std::istream& str, Eigen::Matrix<TT, TS1, TS2>& layer) {
    std::string                line;
    std::getline(str, line);

    std::stringstream          lineStream(line);
    std::string                cell;

    std::getline(lineStream, cell, ','); // skip name of the parameter
    std::getline(lineStream, cell, ',');
    unsigned shape1 = std::stoul(cell);
    std::getline(lineStream, cell, ',');
    unsigned shape2 = std::stoul(cell);
    if(shape1 != layer.rows() || shape2 != layer.cols()) {
        std::stringstream msg{};
        msg << "s1:" << shape1 << " s2:" << shape2;
        throw std::invalid_argument(msg.str());
    }

    for(int i=0; i<layer.rows(); i++) {
        for(int j=0; j<layer.cols(); j++) {
            std::getline(lineStream, cell, ',');
            layer(i, j) = std::stof(cell);
        }
    }
}

NNEvaluator::~NNEvaluator() = default;

NNEvaluator::NNEvaluator(const common::Options& opts)
    : m_opts(opts) {
    m_model = std::make_unique<SGNN>();
    std::ifstream istr("/home/iaroslav/src/sparsegrid/sgtrain/sgw_native.nn");
    if (!istr.good()) {
        istr.close();
        throw std::runtime_error("missing nn file");
    }

    std::string content((std::istreambuf_iterator<char>(istr)), std::istreambuf_iterator<char>());
    std::string decompressed = gzip::decompress(content.data(), content.size());
    std::stringstream ptr{decompressed};
    read_csv(ptr, m_model->l1w);
    read_csv(ptr, m_model->l1b);
    read_csv(ptr, m_model->l2w);
    read_csv(ptr, m_model->l2b);
    read_csv(ptr, m_model->outw);
    read_csv(ptr, m_model->outb);
    istr.close();
}

Score NNEvaluator::evaluate(const brd::BoardState& state) noexcept {
    double res = m_model->forward(state.getNNL());
//    return static_cast<Score>(res * SCORE_SCALE_FACTOR) - (SCORE_SCALE_FACTOR/2);
    return static_cast<Score>(SCORE_SCALE_FACTOR * (2.0 * res - 1.0)/2.0);
}

Score MaterialEvaluator::evaluate(const brd::BoardState& state) noexcept {
    Score scoreF = state.nonPawnMaterial(m_opts.EngineSide);
    Score scoreS = state.nonPawnMaterial(invert(m_opts.EngineSide));
    return scoreF - scoreS;
}





} // namespace eval
