#ifndef INCLUDE_UCI_PROTOCOL_H_
#define INCLUDE_UCI_PROTOCOL_H_
#include "fen.h"

namespace sg { template<typename> class Engine; }
namespace common { struct Options; }
namespace uci {


template<typename TE>
class Protocol {
public:
    explicit Protocol(sg::Engine<TE>& engine, std::istream& is, std::ostream& os, common::Options& opts, uci::Fen& fen) noexcept;

    void ready();
    void stop();
    void accept(std::string_view input);

    void welcomeMsg() noexcept;
    // void info(const search::str::info& info) noexcept;


private:
    std::istream&       m_is;
    std::ostream&       m_os;
    Fen&                m_fen;
    sg::Engine<TE>&     m_engine;
    bool                m_debug;
    bool                m_run;
    bool                m_ponder;
    common::Options&    m_opts;
};

} // namespace uci
#endif  // INCLUDE_UCI_PROTOCOL_H_
