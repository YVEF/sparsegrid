#include "protocol.h"
#include "../search/tm.h"
#include <cassert>
#include "../core/defs.h"
#include "../core/CallerThreadExecutor.h"
#include "../engine.h"


#define UCI_BUF_SZ_CMD 512
namespace uci {

template<std::size_t N>
static inline
bool cmp(std::string_view& input, const char(&lit)[N]) {
    if(input.starts_with(lit)) {
        input.remove_prefix(N);
        return true;
    }
    return false;
}

SQ readSq(std::string_view& input) {
    BB mask = 0x00;
    switch (input[0]) {
        case 'a': mask = NFile::fA; break;
        case 'b': mask = NFile::fB; break;
        case 'c': mask = NFile::fC; break;
        case 'd': mask = NFile::fD; break;
        case 'e': mask = NFile::fE; break;
        case 'f': mask = NFile::fF; break;
        case 'g': mask = NFile::fG; break;
        default: mask = NFile::fH; break;
    }

    switch (input[1]) {
        case '1' : mask &= NRank::r1; break;
        case '2' : mask &= NRank::r2; break;
        case '3' : mask &= NRank::r3; break;
        case '4' : mask &= NRank::r4; break;
        case '5' : mask &= NRank::r5; break;
        case '6' : mask &= NRank::r6; break;
        case '7' : mask &= NRank::r7; break;
        default: mask &= NRank::r8; break;
    }

    input.remove_prefix(2);
    return popLsb(mask);
}


static void handle_go(std::string_view& input, auto& engine, auto& ostream);
static void handle_position(std::string_view& input, auto& engine, auto& fen);
static void handle_option(std::string_view& input, auto& engine);
static void handle_register(std::string_view&);
static void hndl_uci(std::string_view&, auto& ostream);
static void do_quit(auto& run);
static void do_stop(search::TimeManager&);
static void do_ready(auto& ostream);
static void stop(auto& run);


template <typename TE>
void Protocol<TE>::ready() {
    char buf[UCI_BUF_SZ_CMD];
    m_run = true;
    while(m_run) {
        m_is.getline(buf, UCI_BUF_SZ_CMD);
        accept(buf);
    }
}

template <typename TE>
void Protocol<TE>::accept(std::string_view input) {
    if(cmp(input, "position")) [[likely]] handle_position(input, m_engine, m_fen);
    else if (cmp(input, "go")) [[likely]] handle_go(input, m_engine, m_os);
    else if (cmp(input, "ucinewgame")) m_engine.initNewGame(PColor::B);
    else if (cmp(input, "isready")) do_ready(m_os);
    else if (cmp(input, "setoption")) handle_option(input, m_opts);
    else if (cmp(input, "uci")) hndl_uci(input, m_os);
    else if (cmp(input, "debug")) m_debug = cmp(input, "on");
    else if (cmp(input, "register")) handle_register(input);
    else if (cmp(input, "stop")) do_stop(m_engine.tm());
    else if (cmp(input, "quit")) do_quit(m_run);
    else if (cmp(input, "d")) m_engine.printDbg(m_os);
    else if (input.empty() || input[0] == EOF) do_quit(m_run);
}

static unsigned long long readNextTime(std::string_view& input) {
    unsigned long long wtime = 0;
    std::size_t i=0;
    for(; i<input.size() && std::isdigit(input[i]); i++) {
        wtime *= 10;
        wtime += static_cast<uint64_t>(input[i] - '0');
    }
    input.remove_prefix(std::min(i+1, input.size()));
    return wtime;
}

// template<typename TE>
void handle_go(std::string_view& input, auto& engine, auto& ostream) {
    bool res = cmp(input, "wtime");
    assert(res);
    uint64_t wtime = readNextTime(input);

    res = cmp(input, "btime");
    assert(res);
    uint64_t btime = readNextTime(input);

    res = cmp(input, "winc");
    assert(res);
    uint64_t winc = readNextTime(input);

    res = cmp(input, "binc");
    assert(res);
    uint64_t binc = readNextTime(input);

    engine.go(wtime + winc, btime + binc,
        [&ostream](auto res) { 
            ostream << "bestmove " << res.bestmove << " ponder " << res.ponder << std::endl;
        });
}

// todo: hide state under engine
void handle_position(std::string_view& input, auto& engine, auto& fen) {
    if(cmp(input, "fen")) {
        std::size_t cnt = fen.apply(input, engine.state());
        input.remove_prefix(cnt);
    }
    else cmp(input, "startpos");

    if(!cmp(input, "moves")) {
        engine.setupNewBoard(PColor::W);
        return;
    }

    // check the move list
    // rollback, or apply if needed
    std::size_t expect_posit = static_cast<std::size_t>(engine.state().ply()) * 5;
    std::size_t i = std::min(expect_posit, input.size() - 4);
    while(i < expect_posit) {
        engine.state().undo();
        expect_posit -= 5;
    }
    while(i < input.size()) {
        auto from = readSq(input);
        auto to = readSq(input);
        engine.move(from, to);
        i += 5;
    }
}

void handle_option(std::string_view& input, auto& options) {
    cmp(input, "name");
    if(cmp(input, "Nullmove")) {
        cmp(input, "value");
        cmp(input, "true");
    }
    else if(cmp(input, "OwnBook")) {
        cmp(input, "value");
        cmp(input, "true"); // fixme: ignore
    }
    else if(cmp(input, "Ponder")) {
        cmp(input, "value");
        cmp(input, "true"); // fixme: ignore
    }
    else if(cmp(input, "Threads")) {
        cmp(input, "value");
        int i = 0;
        unsigned cores = 0;
        while(std::isdigit(input[i])) {
            cores *= 10;
            cores += (input[i++] - '0');
        }
        options.Cores = cores;
    }
}

void handle_register(std::string_view&) {

}

void hndl_uci(std::string_view&, auto& ostream) {
    ostream << "id name SparseGrid\n"
                 "id author Iaroslav Babanin\n"
                 // "option name Nullmove type check default true\n"
                 "option name Ponder type check default false\n"
                 "option name OwnBook type check default false\n"
                 "option name Threads type spin default 1 min 1 max 32\n"
                 "option name Hash type spin default 2 min 1 max 32768\n"
                 "uciok\n";
}

void do_quit(auto& run) {
    run = false;
    exit(0);
}

void do_stop(search::TimeManager& tm) {
    tm.stop();
}

void do_ready(auto& ostream) {
    ostream << "readyok\n";
}

void stop(auto& run) {
    run = false;
}


template <typename TE>
Protocol<TE>::Protocol(sg::Engine<TE>& engine, std::istream& is, 
        std::ostream& os, common::Options& opts, uci::Fen& fen) noexcept
: m_is(is), m_os(os), m_fen(fen), m_engine(engine), m_opts(opts) {

}

template class Protocol<exec::CallerThreadExecutor>;


} // namespace uci
