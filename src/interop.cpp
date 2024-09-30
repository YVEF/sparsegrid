#include <boost/python.hpp>
#include "dbg/debugger.h"
#include <iostream>
#include "board/board_state.h"
#include "common/options.h"
#include <tuple>
#include <boost/python/numpy.hpp>
#include "eval/evaluator.h"

namespace np = boost::python::numpy;
namespace python = boost::python;

#define OBJECT_NAME(obj) #obj


namespace interop {
class CDC;
void welcome(interop::CDC*) {
    std::cout << "Welcome to SgTrain Interop\n";
}

struct CDCMove {
    uint8_t from;
    uint8_t to;
    uint8_t castling; // preserve castling field for the fast mapping
    bool isEnpass;
    bool isNAM() { return !from && !to && !castling; }
};

class MoveCollection {
public:
    int size() const noexcept { return m_size; }
    interop::CDCMove getMove(int i) const noexcept { return m_data[i]; }
    void clearBag() noexcept { m_size = 0;}
    void push(CDCMove move) noexcept { m_data[m_size++] = move; }
private:
    int m_size = 0;
    interop::CDCMove m_data[64];
};

/*
 * Cross domain communicator
 */
class CDC {
public:
    explicit CDC(brd::BoardState&& state, common::Options opts) noexcept
    : m_state(std::move(state)), m_opts(std::move(opts)) {}

public:
    brd::BoardState m_state;
    common::Options m_opts;
    interop::MoveCollection m_mvCollection{};
};
} // namespace interop

interop::CDC* initCDC() noexcept {
    common::Options opts{};
    opts.NNStateFile = std::string(NN_GZIP_PRETRAINED_WEIGHTS);
    return new interop::CDC(brd::BoardState{brd::Board{}}, std::move(opts));
}

auto convert_(const brd::Move& move) noexcept {
    interop::CDCMove cdcMove{};
    cdcMove.from = move.from;
    cdcMove.to = move.to;
    cdcMove.castling = move.castling;
    cdcMove.isEnpass = move.isEnpass;
    if (move.castling)
        cdcMove.to = CASTL_TO_UCI_CASTL(move.castling, move.from);
    return cdcMove;
}

auto convert_(const interop::CDCMove& cdcMove) noexcept {
    brd::Move move{};
    move.from = cdcMove.from;
    move.to = cdcMove.to;
    move.castling = cdcMove.castling;
    move.isEnpass = cdcMove.isEnpass;
    if (move.castling) move.to = 0x00;

    return move;
}

static void fillNNLayer(const brd::BoardState::nnLayer_t& nnl, const np::ndarray& input) {
    auto data = reinterpret_cast<double*>(input.get_data());
    std::memcpy(data, nnl.data(), brd::nnLayerSize() * sizeof(double));
}

void makeMove(interop::CDC* cdc, const interop::CDCMove& cdcMove, const np::ndarray& input) {
    auto move = convert_(cdcMove);
    cdc->m_state.registerMove(move);
    fillNNLayer(cdc->m_state.getNNL(), input);
}

void undoMove(interop::CDC* cdc, const np::ndarray& input) {
    cdc->m_state.undo();
    fillNNLayer(cdc->m_state.getNNL(), input);
}

void makeMoveSilently(interop::CDC* cdc, const interop::CDCMove& cdcMove) {
    auto move = convert_(cdcMove);
    cdc->m_state.registerMove(move);
}

void undoMoveSilently(interop::CDC* cdc) {
    cdc->m_state.undo();
}

void freeCDC(interop::CDC* cdc) {
    delete cdc;
}

    void displayBoard(interop::CDC* cdc) {
    Debugger::printBB(cdc->m_state);
}

interop::MoveCollection* nextMoves(interop::CDC* cdc, bool color) noexcept {
    cdc->m_mvCollection.clearBag();

    brd::MoveList mvlist{};
    if (color) cdc->m_state.movegenFor<PColor::W>(mvlist);
    else cdc->m_state.movegenFor<PColor::B>(mvlist);

    while (mvlist.size()) {
        auto move = mvlist.pop();
        interop::CDCMove cdcMove = convert_(move);
        cdc->m_mvCollection.push(cdcMove);
    }
    return &cdc->m_mvCollection;
}

interop::CDCMove recognizeMove(const interop::CDC* cdc, uint8_t from, uint8_t to) {
    auto move = recognizeMove(from, to, cdc->m_state.getBoard());
    return convert_(move);
}

np::ndarray initNNLayer() {
    constexpr auto sz= brd::nnLayerSize();
    Py_intptr_t shape[1] = { sz };
    np::ndarray result = np::zeros(1, shape, np::dtype::get_builtin<double>());
    return result;
}

bool isDraw(interop::CDC* cdc) noexcept {
    return cdc->m_state.draw();
}

double getRawEvaluation(interop::CDC* cdc) noexcept {
    eval::NNEvaluator evalu(cdc->m_opts);
    return evalu.evaluateRaw(cdc->m_state);
}


// ======== MAPPING
BOOST_PYTHON_MODULE(sg_trainer_interop) {
    Py_Initialize();
    np::initialize();

    python::class_<interop::CDCMove>(OBJECT_NAME(CDCMove), python::no_init)
        .add_property("fromSq", python::make_getter(&interop::CDCMove::from))
        .add_property("toSq", python::make_getter(&interop::CDCMove::to))
        .add_property("castling", python::make_getter(&interop::CDCMove::castling))
        .add_property("isEnpass", python::make_getter(&interop::CDCMove::isEnpass));

    python::class_<interop::CDC, boost::noncopyable>(OBJECT_NAME(CDC), python::no_init);
    python::class_<interop::MoveCollection, boost::noncopyable>(OBJECT_NAME(MoveCollection), python::no_init)
        .add_property("size", python::make_function(&interop::MoveCollection::size))
        .def("getMove", &interop::MoveCollection::getMove, python::return_value_policy<python::return_by_value>());

    python::def(OBJECT_NAME(initCDC), initCDC, python::return_value_policy<python::reference_existing_object>());
    python::def(OBJECT_NAME(nextMoves), nextMoves, python::return_value_policy<python::reference_existing_object>());

    python::def(OBJECT_NAME(welcome), interop::welcome);
    python::def(OBJECT_NAME(makeMove), makeMove);
    python::def(OBJECT_NAME(undoMove), undoMove);
    python::def(OBJECT_NAME(makeMoveSilently), makeMoveSilently);
    python::def(OBJECT_NAME(undoMoveSilently), undoMoveSilently);
    python::def(OBJECT_NAME(freeCDC), freeCDC);
//    python::def(OBJECT_NAME(fillNNLayer), fillNNLayer);
    python::def(OBJECT_NAME(initNNLayer), initNNLayer);
    python::def(OBJECT_NAME(recognizeMove), recognizeMove);
    python::def(OBJECT_NAME(displayBoard), displayBoard);
    python::def(OBJECT_NAME(isDraw), isDraw);
    python::def(OBJECT_NAME(getRawEvaluation), getRawEvaluation);
}
