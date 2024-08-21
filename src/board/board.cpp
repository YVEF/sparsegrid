#include "board.h"
#include <bit>
#include <cstdint>
#include "../dbg/sg_assert.h"
#include "move.h"
#include "movegen.h"
#include "../core/gens.h"
#include "board_state.h"
#include "../dbg/debugger.h"


namespace brd {


// use Zobrist hashing to support the board stamp
struct BrdZobristSource_ {
    uint64_t map[BRD_SIZE][TOTAL_PKIND_NUM];
    uint64_t blackToMove;
    uint64_t castling[2];
    uint64_t enpassant;
};

static constexpr BrdZobristSource_ zobristSrc = [] {
    BrdZobristSource_ map{};
    gen::rand rnd{};
    auto seq = rnd.gen_sequence_u64<(BRD_SIZE * TOTAL_PKIND_NUM)+1+2+BRD_SIZE, 17317>();
    std::size_t idx=0;
    for(auto& i : map.map) for(auto& j : i) j = seq[idx++];

    map.blackToMove = 1221;
    map.castling[0] = seq[idx++];
    map.castling[1] = seq[idx++];
    map.enpassant = seq[idx++];

    return map;
}();

inline static void xorKey(BrdKey_t& key, PColor color, PKind kind, SQ sq) noexcept {
    SG_ASSERT(kind != PKind::None);
    auto kindId = (int)kind + (6 * static_cast<int>(color));
    key ^= zobristSrc.map[sq][kindId];
}

static void initKey(BrdKey_t& key, Board& board) {
    for (SQ i=0; i<BRD_SIZE; i++) {
        if (board.empty(i)) continue;
        if (!board.empty(i)) {
            BB mask = 1ull << i;
            PKind kind = board.getKind(mask);
            PColor color = board.getColor(mask);
            xorKey(key, color, kind, i);
        }
    }
}

brd::Board::Board() noexcept {
    initKey(m_key, *this);
}


brd::Board& Board::operator=(const Board&) noexcept {
    return *this;
}

bool Board::empty(SQ sq) const noexcept {
    return emptyM(1ull << sq);
}

bool Board::emptyM(BB mask) const noexcept {
    if(m_bb_col & mask) return false;
    return !((m_bb_rqk & mask) || (m_bb_nbk & mask) || (m_bb_pbq & mask));
}


template <PColor Color, PKind Kind>
BB brd::Board::getPieceSqMask() const noexcept {
    auto col = m_bb_col;
    auto odd = m_bb_pbq ^ m_bb_nbk ^ m_bb_rqk;

    if constexpr (Color) { col = ~col; }

    if constexpr (Kind == PKind::pQ) { return m_bb_pbq & m_bb_rqk & col; }
    else if constexpr (Kind == PKind::pR) { return m_bb_rqk & odd & col; }
    else if constexpr (Kind == PKind::pB) { return m_bb_pbq & m_bb_nbk & col; }
    else if constexpr (Kind == PKind::pP) { return m_bb_pbq & odd & col; }
    else if constexpr (Kind == PKind::pK) { return m_bb_nbk & m_bb_rqk & col; }
    else if constexpr (Kind == PKind::pN) { return m_bb_nbk & odd & col; }

    return 0;
}


PColor Board::getColor(BB mask) const noexcept {
    return static_cast<PColor>(!(mask & m_bb_col));
}

PKind Board::getKind(BB mask) const noexcept {
    if(m_bb_nbk & mask) {
        if(m_bb_rqk & mask) return PKind::pK;
        if(m_bb_pbq & mask) return PKind::pB;
        return PKind::pN;
    }
    if(m_bb_rqk & mask) {
        if(m_bb_pbq & mask) return PKind::pQ;
        return PKind::pR;
    }
    if (m_bb_pbq & mask) return PKind::pP;

    return PKind::None;
}

inline static BB slideTo_(BB bb, BB fromMask, BB toMask) {
    return (bb & ~fromMask) | toMask;
}

PKind Board::slideTo(SQ from, SQ to) noexcept {
    BB fromMask = 1ull << from;
    BB toMask = 1ull << to;
    PKind kind = slideToM_(fromMask, toMask);

    PColor color = getColor(toMask);
    xorKey(m_key, color, kind, from);
    xorKey(m_key, color, kind, to);

    return kind;
}

PKind Board::slideToM_(BB fromMask, BB toMask) noexcept {
    auto kind = getKind(fromMask);

    // === ASSERTIONS
    if (empty(std::countr_zero(fromMask))) {
        Debugger::printBB(*this);
        std::cout << std::countr_zero(fromMask) << std::endl;
        int a = 5;
        std::cin >> a;
    }
    SG_ASSERT(!empty(std::countr_zero(fromMask)));
    SG_ASSERT(empty(std::countr_zero(toMask)));
    SG_ASSERT(std::popcount(fromMask) == 1);
    SG_ASSERT(std::popcount(toMask) == 1);
    // === !ASSERTIONS

    if (kind == PKind::pK) {
        m_bb_rqk = slideTo_(m_bb_rqk, fromMask, toMask);
        m_bb_nbk = slideTo_(m_bb_nbk, fromMask, toMask);
    }
    else if (kind == PKind::pP)
        m_bb_pbq = slideTo_(m_bb_pbq, fromMask, toMask);
    else if (kind == PKind::pQ) {
        m_bb_pbq = slideTo_(m_bb_pbq, fromMask, toMask);
        m_bb_rqk = slideTo_(m_bb_rqk, fromMask, toMask);
    }
    else if (kind == PKind::pR)
        m_bb_rqk = slideTo_(m_bb_rqk, fromMask, toMask);
    else if (kind == PKind::pN)
        m_bb_nbk = slideTo_(m_bb_nbk, fromMask, toMask);
    else if (kind == PKind::pB) {
        m_bb_pbq = slideTo_(m_bb_pbq, fromMask, toMask);
        m_bb_nbk = slideTo_(m_bb_nbk, fromMask, toMask);
    }
    else
        m_bb_pbq = slideTo_(m_bb_pbq, fromMask, toMask);

    if (m_bb_col & fromMask) { m_bb_col = (m_bb_col & ~fromMask) | toMask; }

    return kind;
}

std::pair<PColor, PKind> Board::kill(SQ sq) noexcept {
    BB sqMask = uint64_t(1) << sq;
    SG_ASSERT(!emptyM(sqMask));

    PColor col = static_cast<PColor>(!(m_bb_col & sqMask));
    PKind kind = getKind(sqMask);

    m_bb_nbk &= ~sqMask;
    m_bb_pbq &= ~sqMask;
    m_bb_rqk &= ~sqMask;
    m_bb_col &= ~sqMask;

    xorKey(m_key, col, kind, sq);
    return {col, kind};
}


uint64_t Board::key() const noexcept {
    return m_key;
}




template <PColor Color, PKind Kind>
void Board::movegen(MoveList& mvList, const BoardState& state) const noexcept {
    uint64_t enemyMask = m_bb_col;
    if constexpr (!Color) enemyMask = ~enemyMask;

    const BB occupied = occupancy();
    auto pcMask = getPieceSqMask<Color, Kind>();

    while(pcMask) {
        SQ from = std::countr_zero(pcMask);
        pcMask &= (pcMask - 1);
        uint64_t qMoves, aMoves;
        if constexpr (Kind == PKind::pP) {
            qMoves = movegen::getPawnMoves<Color>(from, state);
            aMoves = movegen::getPawnAttacks<Color>(from);
            auto toEnpass = movegen::getEnpassantAttack<Color>(from, state);
            if(toEnpass)
                mvList.push(brd::mkEnpass(from, toEnpass));
        }
        else
            qMoves = aMoves = movegen::getPieceOccupancy<Color, Kind>(from, occupied);

        if(!qMoves && !aMoves) continue;

        // quiet and attack moves
        auto qm = qMoves & ~occupied;
        auto am = aMoves & occupied & enemyMask;
        auto moves = qm | am;
        while(moves) {
            mvList.push(brd::mkMove(from, std::countr_zero(moves)));
            moves &= (moves - 1);
        }

        if constexpr (Kind == PKind::pK) {
            if (state.kindNotMoved<Color>())
                movegen::genCastling<Color>(from, mvList, state);
        }
    }
}


void Board::put(PKind kind, PColor color, SQ sq) noexcept {
    SG_ASSERT(kind != PKind::None);

    BB mask = 1ull << sq;
    switch(kind) {
        case PKind::pQ: {
                            m_bb_rqk |= mask;
                            m_bb_pbq |= mask;
                            break;
                        }
        case PKind::pB: {
                            m_bb_nbk |= mask;
                            m_bb_pbq |= mask;
                            break;
                        }
        case PKind::pK: {
                            m_bb_rqk |= mask;
                            m_bb_nbk |= mask;
                            break;
                        }


        case PKind::pN: {
                            m_bb_nbk |= mask;
                            break;
                        }
        case PKind::pR: {
                            m_bb_rqk |= mask;
                            break;
                        }

        default: {
                     m_bb_pbq |= mask;
                     break;
                 }
    }

    if (!color) m_bb_col |= mask;

    xorKey(m_key, color, kind, sq);
}

TEMPLATE_DEF_CONST(uint64_t, brd::Board::getPieceSqMask)
TEMPLATE_DEF_CONST(void, brd::Board::movegen, MoveList&, const BoardState&)

void Board::updateKey(uint8_t castling, bool isEnpass) noexcept {
    m_key ^= zobristSrc.blackToMove;
    if (castling) {
        SG_ASSERT(castling <= 0x02);
        m_key ^= zobristSrc.castling[castling];
    }
    else if (isEnpass)
        m_key ^= zobristSrc.enpassant;
}

uint64_t Board::stateKey() const noexcept {
    auto shiftKey = m_bb_col >> 16;
    shiftKey ^= (m_bb_nbk >> 32);
    shiftKey ^= (m_bb_pbq << 16);
    shiftKey ^= (m_bb_rqk << 32);
    return shiftKey;
}

void Board::clear() noexcept {
    m_bb_rqk = m_bb_pbq = m_bb_nbk = m_bb_col = 0x00;
}

void Board::rebuildKey() noexcept {
    initKey(m_key, *this);
    updateKey(0x00, false);
}

std::tuple<uint64_t, uint64_t, uint64_t, uint64_t> Board::rawBoard() const noexcept {
    return std::make_tuple(m_bb_col, m_bb_nbk, m_bb_pbq, m_bb_rqk);
}

} // namespace brd
