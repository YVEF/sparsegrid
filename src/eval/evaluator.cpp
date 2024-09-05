#include "evaluator.h"
#include "../common/options.h"
#include "../board/board_state.h"
#include <Eigen/Dense>
#include <memory>
#include <gzip/decompress.hpp>
#include <fstream>
#include <string>
#include <unsupported/Eigen/CXX11/Tensor>
#include "../dbg/sg_assert.h"

// todo:
//  1. use float
//  2. rewrite with simd
namespace eval {
template<int S1, int S2, int TOps, int S3, int S4, typename T>
static Eigen::Matrix<T, S1, S2> mapInto(const Eigen::Matrix<T, S3, S4>& input) noexcept {
    return Eigen::Matrix<T, S1, S2, TOps>(input.data());
}

constexpr static int input_channels = brd::nnLayerSize();
constexpr static int output_channels = 128;
constexpr static int batch_number = 40;
constexpr static int kernel_size = input_channels / batch_number;

typedef Eigen::TensorFixedSize<double, Eigen::Sizes<output_channels, batch_number, kernel_size>> Conv1d_t;
typedef Eigen::Matrix<double, 1, input_channels> Input_t;

Eigen::Matrix<double, 1, output_channels> conv1d(
    const Input_t& raw_input,
    const Conv1d_t& weights) noexcept {

    auto input = mapInto<batch_number, kernel_size, Eigen::RowMajor>(raw_input);
    auto output = Eigen::Matrix<double, output_channels, 1>();
    output.setZero();

    for (int out_ch = 0; out_ch < output_channels; out_ch++)
        for (int bs = 0; bs < batch_number; bs++)
            for (int k=0; k<kernel_size; k++)
                output(out_ch, 0) += input(bs, k) * weights(out_ch, bs, k);

    return output.transpose();
}

class SGNN {
public:
    Conv1d_t conv1dl{};
    Eigen::Matrix<double, 64, 128> lin2w{};
    Eigen::Vector<double, 64> lin2b{};
    Eigen::Matrix<double, 1, 64> outw{};
    Eigen::Vector<double, 1> outb{};

    [[nodiscard]] double forward(const brd::BoardState::nnLayer_t& input) const noexcept {
        Eigen::Matrix<double, 1, input_channels> nnState(input.data());
        auto s1 = conv1d(nnState, conv1dl);
        auto s2 = s1.unaryExpr(&LeakyReLU01);
        auto s3 = s2 * lin2w.transpose();
        auto s4 = s3 + lin2b.transpose();
        auto s5 = s4.unaryExpr(&LeakyReLU03);
        auto s6 = s5 * outw.transpose();
        auto s7 = s6 + outb.transpose();
        auto s8 = s7.unaryExpr(&Sigmoid);

        return s8(0,0);
    }

    template<double Slope> static double LeakyReLU(double x) noexcept {
        return (x >= 0.0) ? x : Slope * x;
    }
    static double Sigmoid(double x) noexcept {
        return 1.0/(1.0 + std::exp(-x));
    }
    static double LeakyReLU01(double x) noexcept { return LeakyReLU<0.1>(x); }
    static double LeakyReLU03(double x) noexcept { return LeakyReLU<0.3>(x); }
};

void read_csv(std::istream& str, auto&& callback, int dims = 2) {
    std::string line, cell;
    std::getline(str, line);
    std::stringstream lineStream(line);

    std::getline(lineStream, cell, ','); // skip name of the parameter
    for (int i=0; i<dims; i++) {
        std::getline(lineStream, cell, ',');
        auto _ = std::stoul(cell);
    }

    callback(lineStream);

    SG_ASSERT(!std::getline(lineStream, cell, ',') || cell.empty());
}

template<typename TT, typename TSizes>
void read_into(std::stringstream& lineStream, Eigen::TensorFixedSize<TT, TSizes>& layer) noexcept {
    std::string cell;
    for (int i=0; i<layer.dimension(0); i++)
        for (int j=0; j<layer.dimension(1); j++)
            for (int k=0; k<layer.dimension(2); k++) {
                std::getline(lineStream, cell, ',');
                layer(i, j, k) = std::stod(cell);
            }
}

template<typename TT, int TS1, int TS2>
void read_into(std::stringstream& lineStream, Eigen::Matrix<TT, TS1, TS2>& layer) noexcept {
    std::string cell;
    for(int i=0; i<layer.rows(); i++) {
        for(int j=0; j<layer.cols(); j++) {
            std::getline(lineStream, cell, ',');
            layer(i, j) = std::stod(cell);
        }
    }
}

NNEvaluator::~NNEvaluator() = default;
NNEvaluator::NNEvaluator(const common::Options& opts) : m_opts(opts) {
    m_model = std::make_unique<SGNN>();
    std::ifstream istr(opts.NNStateFile);
    if (!istr.good()) {
        istr.close();
        throw std::runtime_error(".nn file not found");
    }

    std::string content((std::istreambuf_iterator<char>(istr)), std::istreambuf_iterator<char>());
    std::string decompressed = gzip::decompress(content.data(), content.size());
    std::stringstream ptr{decompressed};

    read_csv(ptr,
             [&layer = m_model->conv1dl](std::stringstream& ls) { read_into(ls, layer); },
             3);

    read_csv(ptr, [&layer = m_model->lin2w](std::stringstream& ls) { read_into(ls, layer); });
    read_csv(ptr, [&layer = m_model->lin2b](std::stringstream& ls) { read_into(ls, layer); });
    read_csv(ptr, [&layer = m_model->outw](std::stringstream& ls) { read_into(ls, layer); });
    read_csv(ptr, [&layer = m_model->outb](std::stringstream& ls) { read_into(ls, layer); });

    istr.close();
}

Score NNEvaluator::evaluate(const brd::BoardState& state) noexcept {
    double res = evaluateRaw(state);
    return static_cast<Score>(SCORE_SCALE_FACTOR * (2.0 * res - 1.0)/2.0);
}

inline double NNEvaluator::evaluateRaw(const brd::BoardState& state) noexcept {
    return m_model->forward(state.getNNL());
}

Score MaterialEvaluator::evaluate(const brd::BoardState& state) noexcept {
    Score scoreF = state.nonPawnMaterial(m_opts.EngineSide);
    Score scoreS = state.nonPawnMaterial(invert(m_opts.EngineSide));
    return scoreF - scoreS;
}



} // namespace eval
