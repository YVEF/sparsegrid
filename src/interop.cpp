#include <boost/python.hpp>
#include <iostream>
#include "board/board_state.h"
#include "common/options.h"
#include <tuple>
//#include <boost/numpy.hpp>
#include <boost/python/numpy.hpp>

namespace np = boost::python::numpy;
//namespace np = boost::numpy;
namespace python = boost::python;

#define OBJECT_NAME(obj) #obj

char const* greet(int i) {
    return "hello, world";
}



namespace interop {
struct CDCMove {
    uint8_t from;
    uint8_t to;
    uint8_t castling;
    bool isEnpassant;
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
    : m_state(std::move(state)), m_opts(opts)//, m_mvCollection(std::make_shared<interop::MoveCollection>())
    {}

public:
    brd::BoardState m_state;
    common::Options m_opts;
//    std::shared_ptr<interop::MoveCollection> m_mvCollection;
    interop::MoveCollection m_mvCollection{};
};


} // namespace interop

interop::CDC* initCDC(bool color) noexcept {
    common::Options opts{};
    opts.EngineSide = static_cast<PColor>(color);
    return new interop::CDC(brd::BoardState{brd::Board{}}, opts);
}


void makeMove(interop::CDC* cdc, const brd::Move& move) noexcept {
    cdc->m_state.registerMove(move);
}

void undoMove(interop::CDC* cdc) noexcept {
    cdc->m_state.undo();
}

void freeCDC(interop::CDC* cdc) {
    delete cdc;
}

interop::MoveCollection* nextMoves(interop::CDC* cdc, bool color) {
    cdc->m_mvCollection.clearBag();
    std::cout << "reset done\n";

    brd::MoveList mvlist{};
    if (color) cdc->m_state.movegenFor<PColor::W>(mvlist);
    else cdc->m_state.movegenFor<PColor::B>(mvlist);

    while (mvlist.size()) {
        auto move = mvlist.pop();
        interop::CDCMove cdcMove{};
        cdcMove.from = move.from;
        cdcMove.to = move.to;
        cdcMove.castling = move.castling;
        cdcMove.isEnpassant = move.isEnpass;
        cdc->m_mvCollection.push(cdcMove);
    }
    return &cdc->m_mvCollection;
}

void welcome(interop::CDC* cdc) {
    std::cout << "welcome\n";
}

static constexpr std::size_t nnInputSize() {
    using rawBoardTuple = decltype(std::declval<brd::BoardState>().getBoard().rawBoard());
    using rawBoardPart = decltype(std::get<0>(rawBoardTuple{}));
    constexpr auto r = sizeof(rawBoardPart) * 8 * (std::tuple_size_v<rawBoardTuple> + 1);
    static_assert(r == 320);
    return r;
}

np::ndarray initNNInputLayer() {
    std::cout << "1\n";
    constexpr auto sz= nnInputSize();
    Py_intptr_t shape[1] = { sz };
    std::cout << "2\n";
    auto kk = np::dtype::get_builtin<double>();
    std::cout << "3\n";
    np::ndarray result = np::zeros(1, shape, kk);
    std::cout << "4\n";

    return result;
}



template<std::size_t I>
static void fillBits(double* input, const auto& rawBrd) {
    auto brdPart = std::get<I>(rawBrd);
    constexpr auto start = I*64;
    for (SQ i=0; i<64; i++) {
        if ((1ull << i) & brdPart) input[start+i] = 1.0;
    }
}

// todo: rewrite to swap only bits
void fillInputLayer(interop::CDC* cdc, double* input) {
    constexpr auto sz = nnInputSize();
    std::fill_n(input, sz, 0.0);

    auto rawBrd = cdc->m_state.getBoard().rawBoard();
    fillBits<0>(input, rawBrd);
    fillBits<1>(input, rawBrd);
    fillBits<2>(input, rawBrd);
    fillBits<3>(input, rawBrd);
    if (getNextPlayerColor(cdc->m_state, cdc->m_opts))
        input[256] = 1.0;
    else
        input[319] = 1.0;
}



// ======== MAPPING
BOOST_PYTHON_MODULE(sg_trainer_interop) {
    Py_Initialize();
    np::initialize();

    python::class_<interop::CDCMove>(OBJECT_NAME(CDCMove), python::no_init)
        .add_property("fromSq", python::make_getter(&interop::CDCMove::from))
        .add_property("toSq", python::make_getter(&interop::CDCMove::to))
        .add_property("castling", python::make_getter(&interop::CDCMove::castling));

    python::class_<interop::CDC, boost::noncopyable>(OBJECT_NAME(CDC), python::no_init);
    python::class_<interop::MoveCollection, boost::noncopyable>(OBJECT_NAME(MoveCollection), python::no_init)
        .add_property("size", python::make_function(&interop::MoveCollection::size))
        .def("getMove", &interop::MoveCollection::getMove, python::return_value_policy<python::return_by_value>());

    python::class_<interop::CDCMove, boost::noncopyable>(OBJECT_NAME(CDCMove), python::no_init)
        .add_property("fromSq", python::make_getter(&interop::CDCMove::from))
        .add_property("toSq", python::make_getter(&interop::CDCMove::to))
        .add_property("castling", python::make_getter(&interop::CDCMove::castling));


    python::def(OBJECT_NAME(initCDC), initCDC, python::return_value_policy<python::reference_existing_object>());
    python::def(OBJECT_NAME(nextMoves), nextMoves, python::return_value_policy<python::reference_existing_object>());

    python::def(OBJECT_NAME(welcome), welcome);
    python::def(OBJECT_NAME(makeMove), makeMove);
    python::def(OBJECT_NAME(undoMove), undoMove);
    python::def(OBJECT_NAME(freeCDC), freeCDC);
    python::def(OBJECT_NAME(fillInputLayer), fillInputLayer);
    python::def(OBJECT_NAME(initNNInputLayer), initNNInputLayer);
}
