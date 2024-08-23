#include "movegen.h"
#include <array>
#include <cstdint>
#include "../core/defs.h"
#include "../dbg/sg_assert.h"
#include "board_state.h"
#include "../dbg/debugger.h"


namespace movegen {
constexpr BB bishop_masks[] {
    0x0040201008040200ULL, 0x0000402010080400ULL, 0x0000004020100a00ULL, 0x0000000040221400ULL,
    0x0000000002442800ULL, 0x0000000204085000ULL, 0x0000020408102000ULL, 0x0002040810204000ULL,
    0x0020100804020000ULL, 0x0040201008040000ULL, 0x00004020100a0000ULL, 0x0000004022140000ULL,
    0x0000000244280000ULL, 0x0000020408500000ULL, 0x0002040810200000ULL, 0x0004081020400000ULL,
    0x0010080402000200ULL, 0x0020100804000400ULL, 0x004020100a000a00ULL, 0x0000402214001400ULL,
    0x0000024428002800ULL, 0x0002040850005000ULL, 0x0004081020002000ULL, 0x0008102040004000ULL,
    0x0008040200020400ULL, 0x0010080400040800ULL, 0x0020100a000a1000ULL, 0x0040221400142200ULL,
    0x0002442800284400ULL, 0x0004085000500800ULL, 0x0008102000201000ULL, 0x0010204000402000ULL,
    0x0004020002040800ULL, 0x0008040004081000ULL, 0x00100a000a102000ULL, 0x0022140014224000ULL,
    0x0044280028440200ULL, 0x0008500050080400ULL, 0x0010200020100800ULL, 0x0020400040201000ULL,
    0x0002000204081000ULL, 0x0004000408102000ULL, 0x000a000a10204000ULL, 0x0014001422400000ULL,
    0x0028002844020000ULL, 0x0050005008040200ULL, 0x0020002010080400ULL, 0x0040004020100800ULL,
    0x0000020408102000ULL, 0x0000040810204000ULL, 0x00000a1020400000ULL, 0x0000142240000000ULL,
    0x0000284402000000ULL, 0x0000500804020000ULL, 0x0000201008040200ULL, 0x0000402010080400ULL,
    0x0002040810204000ULL, 0x0004081020400000ULL, 0x000a102040000000ULL, 0x0014224000000000ULL,
    0x0028440200000000ULL, 0x0050080402000000ULL, 0x0020100804020000ULL, 0x0040201008040200ULL};

constexpr BB rook_masks[] {
    0x000101010101017eULL, 0x000202020202027cULL, 0x000404040404047aULL, 0x0008080808080876ULL,
    0x001010101010106eULL, 0x002020202020205eULL, 0x004040404040403eULL, 0x008080808080807eULL,
    0x0001010101017e00ULL, 0x0002020202027c00ULL, 0x0004040404047a00ULL, 0x0008080808087600ULL,
    0x0010101010106e00ULL, 0x0020202020205e00ULL, 0x0040404040403e00ULL, 0x0080808080807e00ULL,
    0x00010101017e0100ULL, 0x00020202027c0200ULL, 0x00040404047a0400ULL, 0x0008080808760800ULL,
    0x00101010106e1000ULL, 0x00202020205e2000ULL, 0x00404040403e4000ULL, 0x00808080807e8000ULL,
    0x000101017e010100ULL, 0x000202027c020200ULL, 0x000404047a040400ULL, 0x0008080876080800ULL,
    0x001010106e101000ULL, 0x002020205e202000ULL, 0x004040403e404000ULL, 0x008080807e808000ULL,
    0x0001017e01010100ULL, 0x0002027c02020200ULL, 0x0004047a04040400ULL, 0x0008087608080800ULL,
    0x0010106e10101000ULL, 0x0020205e20202000ULL, 0x0040403e40404000ULL, 0x0080807e80808000ULL,
    0x00017e0101010100ULL, 0x00027c0202020200ULL, 0x00047a0404040400ULL, 0x0008760808080800ULL,
    0x00106e1010101000ULL, 0x00205e2020202000ULL, 0x00403e4040404000ULL, 0x00807e8080808000ULL,
    0x007e010101010100ULL, 0x007c020202020200ULL, 0x007a040404040400ULL, 0x0076080808080800ULL,
    0x006e101010101000ULL, 0x005e202020202000ULL, 0x003e404040404000ULL, 0x007e808080808000ULL,
    0x7e01010101010100ULL, 0x7c02020202020200ULL, 0x7a04040404040400ULL, 0x7608080808080800ULL,
    0x6e10101010101000ULL, 0x5e20202020202000ULL, 0x3e40404040404000ULL, 0x7e80808080808000ULL};

constexpr int bishop_shifts[] {
    58, 59, 59, 59, 59, 59, 59, 58,
    59, 59, 59, 59, 59, 59, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 55, 55, 57, 59, 59,
    59, 59, 57, 57, 57, 57, 59, 59,
    59, 59, 59, 59, 59, 59, 59, 59,
    58, 59, 59, 59, 59, 59, 59, 58};

constexpr int rook_shifts[] {
    52, 53, 53, 53, 53, 53, 53, 52,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    53, 54, 54, 54, 54, 54, 54, 53,
    52, 53, 53, 53, 53, 53, 53, 52};

constexpr BB bishop_magics[] {
    0x0002020202020200ULL, 0x0002020202020000ULL, 0x0004010202000000ULL, 0x0004040080000000ULL,
    0x0001104000000000ULL, 0x0000821040000000ULL, 0x0000410410400000ULL, 0x0000104104104000ULL,
    0x0000040404040400ULL, 0x0000020202020200ULL, 0x0000040102020000ULL, 0x0000040400800000ULL,
    0x0000011040000000ULL, 0x0000008210400000ULL, 0x0000004104104000ULL, 0x0000002082082000ULL,
    0x0004000808080800ULL, 0x0002000404040400ULL, 0x0001000202020200ULL, 0x0000800802004000ULL,
    0x0000800400A00000ULL, 0x0000200100884000ULL, 0x0000400082082000ULL, 0x0000200041041000ULL,
    0x0002080010101000ULL, 0x0001040008080800ULL, 0x0000208004010400ULL, 0x0000404004010200ULL,
    0x0000840000802000ULL, 0x0000404002011000ULL, 0x0000808001041000ULL, 0x0000404000820800ULL,
    0x0001041000202000ULL, 0x0000820800101000ULL, 0x0000104400080800ULL, 0x0000020080080080ULL,
    0x0000404040040100ULL, 0x0000808100020100ULL, 0x0001010100020800ULL, 0x0000808080010400ULL,
    0x0000820820004000ULL, 0x0000410410002000ULL, 0x0000082088001000ULL, 0x0000002011000800ULL,
    0x0000080100400400ULL, 0x0001010101000200ULL, 0x0002020202000400ULL, 0x0001010101000200ULL,
    0x0000410410400000ULL, 0x0000208208200000ULL, 0x0000002084100000ULL, 0x0000000020880000ULL,
    0x0000001002020000ULL, 0x0000040408020000ULL, 0x0004040404040000ULL, 0x0002020202020000ULL,
    0x0000104104104000ULL, 0x0000002082082000ULL, 0x0000000020841000ULL, 0x0000000000208800ULL,
    0x0000000010020200ULL, 0x0000000404080200ULL, 0x0000040404040400ULL, 0x0002020202020200ULL
};
constexpr BB rook_magics[] {
    0x0080001020400080ULL, 0x0040001000200040ULL, 0x0080081000200080ULL, 0x0080040800100080ULL,
    0x0080020400080080ULL, 0x0080010200040080ULL, 0x0080008001000200ULL, 0x0080002040800100ULL,
    0x0000800020400080ULL, 0x0000400020005000ULL, 0x0000801000200080ULL, 0x0000800800100080ULL,
    0x0000800400080080ULL, 0x0000800200040080ULL, 0x0000800100020080ULL, 0x0000800040800100ULL,
    0x0000208000400080ULL, 0x0000404000201000ULL, 0x0000808010002000ULL, 0x0000808008001000ULL,
    0x0000808004000800ULL, 0x0000808002000400ULL, 0x0000010100020004ULL, 0x0000020000408104ULL,
    0x0000208080004000ULL, 0x0000200040005000ULL, 0x0000100080200080ULL, 0x0000080080100080ULL,
    0x0000040080080080ULL, 0x0000020080040080ULL, 0x0000010080800200ULL, 0x0000800080004100ULL,
    0x0000204000800080ULL, 0x0000200040401000ULL, 0x0000100080802000ULL, 0x0000080080801000ULL,
    0x0000040080800800ULL, 0x0000020080800400ULL, 0x0000020001010004ULL, 0x0000800040800100ULL,
    0x0000204000808000ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000010002008080ULL, 0x0000004081020004ULL,
    0x0000204000800080ULL, 0x0000200040008080ULL, 0x0000100020008080ULL, 0x0000080010008080ULL,
    0x0000040008008080ULL, 0x0000020004008080ULL, 0x0000800100020080ULL, 0x0000800041000080ULL,
    0x00FFFCDDFCED714AULL, 0x007FFCDDFCED714AULL, 0x003FFFCDFFD88096ULL, 0x0000040810002101ULL,
    0x0001000204080011ULL, 0x0001000204000801ULL, 0x0001000082000401ULL, 0x0001FFFAABFAD1A2ULL
};

constexpr BB king_occupancy[] {
    0x0000000000000302ULL, 0x0000000000000705ULL, 0x0000000000000e0aULL, 0x0000000000001c14ULL,
    0x0000000000003828ULL, 0x0000000000007050ULL, 0x000000000000e0a0ULL, 0x000000000000c040ULL,
    0x0000000000030203ULL, 0x0000000000070507ULL, 0x00000000000e0a0eULL, 0x00000000001c141cULL,
    0x0000000000382838ULL, 0x0000000000705070ULL, 0x0000000000e0a0e0ULL, 0x0000000000c040c0ULL,
    0x0000000003020300ULL, 0x0000000007050700ULL, 0x000000000e0a0e00ULL, 0x000000001c141c00ULL,
    0x0000000038283800ULL, 0x0000000070507000ULL, 0x00000000e0a0e000ULL, 0x00000000c040c000ULL,
    0x0000000302030000ULL, 0x0000000705070000ULL, 0x0000000e0a0e0000ULL, 0x0000001c141c0000ULL,
    0x0000003828380000ULL, 0x0000007050700000ULL, 0x000000e0a0e00000ULL, 0x000000c040c00000ULL,
    0x0000030203000000ULL, 0x0000070507000000ULL, 0x00000e0a0e000000ULL, 0x00001c141c000000ULL,
    0x0000382838000000ULL, 0x0000705070000000ULL, 0x0000e0a0e0000000ULL, 0x0000c040c0000000ULL,
    0x0003020300000000ULL, 0x0007050700000000ULL, 0x000e0a0e00000000ULL, 0x001c141c00000000ULL,
    0x0038283800000000ULL, 0x0070507000000000ULL, 0x00e0a0e000000000ULL, 0x00c040c000000000ULL,
    0x0302030000000000ULL, 0x0705070000000000ULL, 0x0e0a0e0000000000ULL, 0x1c141c0000000000ULL,
    0x3828380000000000ULL, 0x7050700000000000ULL, 0xe0a0e00000000000ULL, 0xc040c00000000000ULL,
    0x0203000000000000ULL, 0x0507000000000000ULL, 0x0a0e000000000000ULL, 0x141c000000000000ULL,
    0x2838000000000000ULL, 0x5070000000000000ULL, 0xa0e0000000000000ULL, 0x40c0000000000000ULL
};

constexpr BB knigth_occupancy[] {
    0x0000000000020400ULL, 0x0000000000050800ULL, 0x00000000000a1100ULL, 0x0000000000142200ULL,
    0x0000000000284400ULL, 0x0000000000508800ULL, 0x0000000000a01000ULL, 0x0000000000402000ULL,
    0x0000000002040004ULL, 0x0000000005080008ULL, 0x000000000a110011ULL, 0x0000000014220022ULL,
    0x0000000028440044ULL, 0x0000000050880088ULL, 0x00000000a0100010ULL, 0x0000000040200020ULL,
    0x0000000204000402ULL, 0x0000000508000805ULL, 0x0000000a1100110aULL, 0x0000001422002214ULL,
    0x0000002844004428ULL, 0x0000005088008850ULL, 0x000000a0100010a0ULL, 0x0000004020002040ULL,
    0x0000020400040200ULL, 0x0000050800080500ULL, 0x00000a1100110a00ULL, 0x0000142200221400ULL,
    0x0000284400442800ULL, 0x0000508800885000ULL, 0x0000a0100010a000ULL, 0x0000402000204000ULL,
    0x0002040004020000ULL, 0x0005080008050000ULL, 0x000a1100110a0000ULL, 0x0014220022140000ULL,
    0x0028440044280000ULL, 0x0050880088500000ULL, 0x00a0100010a00000ULL, 0x0040200020400000ULL,
    0x0204000402000000ULL, 0x0508000805000000ULL, 0x0a1100110a000000ULL, 0x1422002214000000ULL,
    0x2844004428000000ULL, 0x5088008850000000ULL, 0xa0100010a0000000ULL, 0x4020002040000000ULL,
    0x0400040200000000ULL, 0x0800080500000000ULL, 0x1100110a00000000ULL, 0x2200221400000000ULL,
    0x4400442800000000ULL, 0x8800885000000000ULL, 0x100010a000000000ULL, 0x2000204000000000ULL,
    0x0004020000000000ULL, 0x0008050000000000ULL, 0x00110a0000000000ULL, 0x0022140000000000ULL,
    0x0044280000000000ULL, 0x0088500000000000ULL, 0x0010a00000000000ULL, 0x0020400000000000ULL
};

// constexpr BB white_pawn_moves[] {

// };

#define HORIZ_DIR_U 1
#define HORIZ_DIR_D (-1)
#define VERT_DIR_U 8
#define VERT_DIR_D (-8)

#define DIAG_DIR_UR 9
#define DIAG_DIR_UL 7
#define DIAG_DIR_DR -7
#define DIAG_DIR_DL -9

uint64_t rook_attacks[64][4096];
uint64_t bishop_attacks[64][512];

constexpr uint64_t genDirectSliding(uint8_t inputSq, int coeff, uint64_t occupied) {
    int sq = inputSq;
    uint64_t res = 0;

    constexpr uint64_t topBottom = NRank::r1 | NRank::r8;
    constexpr uint64_t leftRight = NFile::fA | NFile::fH;

    if ((1ULL << sq) & NRank::r1 && coeff < -2) {
        return res;
    }

    if ((1ULL << sq) & NRank::r8 && coeff > 2) {
        return res;
    }

    if ((1ULL << sq) & NFile::fA
        && (coeff == -1 || coeff == -9 || coeff == 7)) {
        return res;
    }
    if ((1ULL << sq) & NFile::fH
        && (coeff == 1 || coeff == -7 || coeff == 9)) {
        return res;
    }

    while(true) {
        sq += coeff;
        const uint64_t currSq = 1ULL << sq;
        if(sq < 0 || sq >= 64)
            break;

        res |= currSq;

        if (currSq & occupied)
            break;

        if (std::abs(coeff) == 8) {
            if(currSq & topBottom)
                break;
        }
        else if (std::abs(coeff) == 1) {
            if (currSq & leftRight)
                break;
        }
        else {
            if(currSq & OUTER_SQUARES_BB)
                break;
        }
    }

    return res;
}


constexpr uint64_t generateRookAttacks(uint8_t sq, uint64_t occupied) {
    return genDirectSliding(sq, VERT_DIR_U, occupied) | genDirectSliding(sq, VERT_DIR_D, occupied)
        | genDirectSliding(sq, HORIZ_DIR_U, occupied) | genDirectSliding(sq, HORIZ_DIR_D, occupied);
}

constexpr uint64_t generateBishopAttacks(uint8_t sq, uint64_t occupied) {
    return genDirectSliding(sq, DIAG_DIR_UR, occupied) | genDirectSliding(sq, DIAG_DIR_UL, occupied)
        | genDirectSliding(sq, DIAG_DIR_DL, occupied) | genDirectSliding(sq, DIAG_DIR_DR, occupied);
}



uint64_t getRookOccupancy(SQ sq, uint64_t occupied) noexcept {
    uint64_t occ = occupied & rook_masks[sq];
    occ *= rook_magics[sq];
    occ >>= rook_shifts[sq];
    return rook_attacks[sq][occ];
}

uint64_t getBishopOccupancy(SQ sq, uint64_t occupied) noexcept {
    uint64_t occ = occupied & bishop_masks[sq];
    occ *= bishop_magics[sq];
    occ >>= bishop_shifts[sq];
    return bishop_attacks[sq][occ];
}

uint64_t getKingOccupancy(SQ sq) noexcept {
    return king_occupancy[sq];
}

uint64_t getKnightOccupancy(SQ sq) noexcept {
    return knigth_occupancy[sq];
}

template<PColor Color>
uint64_t getPawnAttacks(SQ sq) noexcept {
    uint64_t maskSq = 1ull << sq;
    SG_ASSERT(!(maskSq & NRank::r8) && !(maskSq & NRank::r1));
    
    uint64_t res = 0;
    constexpr int upRight = Color == PColor::W ? 9 : -7;
    constexpr int upLeft = Color == PColor::W ? 7 : -9;

    if (!(maskSq & NFile::fA)) {
        res |= (1ull << (sq + upLeft));
    }
    if (!(maskSq & NFile::fH))
        res |= (1ull << (sq + upRight));

    return res;
}


// todo: refactor
template<PColor Color>
uint8_t getEnpassantAttack(SQ sq, const brd::BoardState& state) noexcept {
    if(!state.ply()) return 0x00;

    const auto& lmv = state.getLastMove();
    PKind moveKind = static_cast<PKind>(lmv.moveKind);
    SQ from = static_cast<SQ>(lmv.from);
    SQ to = static_cast<SQ>(lmv.to);
    BB toMask = 1ull << to;
    auto&& brd = state.getBoard();
    PColor targetCol = brd.getColor(toMask);

    if(moveKind != PKind::pP 
            || std::abs((int)from - (int)to) != 16 
            || std::abs((int)to - (int)sq) != 1
            || to/8 != sq/8
            || targetCol == Color) return 0x00;

    BB res = 0;
    if constexpr (Color == PColor::W) {
        if (!(toMask & NRank::r5)) return 0x00;
        if (brd.empty(to+8)) res = to+8;
    }
    else {
        if (!(toMask & NRank::r4)) return 0x00;
        if (brd.empty(to-8)) res = to-8;
    }

    return res;
}

// todo: optmize bitscan
template<PColor Color>
uint64_t getPawnMoves(SQ sq, const brd::BoardState& state) noexcept {
    uint64_t maskSq = 1ull << sq;
    SG_ASSERT(!(maskSq & NRank::r8) && !(maskSq & NRank::r1));

    constexpr int up = Color == PColor::W ? 8 : -8;
    if (!state.getBoard().empty(sq + up)) return 0x00;
    uint64_t res = 1ull << (sq + up);

    if constexpr (Color == PColor::W) {
        if (maskSq & NRank::r2)
            res |= 1ull << (sq + 2*up);
    }
    else {
        if (maskSq & NRank::r7)
            res |= 1ull << (sq + 2*up);
    }

    return res;
}







[[nodiscard]] inline uint64_t lsbReset(uint64_t number) {
    return number & (number - 1);
}


inline void setBit(uint64_t& number, uint64_t index) {
    number |= (1ULL << index);
}


[[nodiscard]] inline bool getBit(uint64_t number, uint64_t index) {
    return ((number >> index) & 1ULL) == 1;
}
uint64_t populateMask(uint64_t mask, uint64_t index) {
    uint64_t    res = 0;
    uint64_t i   = 0;
    
    while (mask) {
        const uint64_t bit = std::countr_zero(mask);
        
        if (getBit(index, i)) {
            setBit(res, bit);
        }
        
        mask = lsbReset(mask);
        i++;
    }
    
    return res;
}

void init() {
    for(SQ sq = 0; sq < SQ_CNT; sq++) {
        int rookBitCnt = 64 - rook_shifts[sq];
        int bishopBitCnt = 64 - bishop_shifts[sq];
        const uint64_t rookEntries = 1ULL << rookBitCnt;
        const uint64_t bishopEntries = 1ULL << bishopBitCnt;
        SG_ASSERT(rookEntries <= std::size(rook_attacks[sq]), rookEntries, std::size(rook_attacks[sq]));
        SG_ASSERT(bishopEntries <= std::size(bishop_attacks[sq]), bishopEntries, std::size(bishop_attacks[sq]));

        for(uint64_t ent = 0; ent < rookEntries; ent++) {
            uint64_t occ = populateMask(rook_masks[sq], ent);
            auto index = (occ * rook_magics[sq]) >> rook_shifts[sq];
            rook_attacks[sq][index] = generateRookAttacks(sq, occ);
        }

        for(uint64_t ent = 0; ent < bishopEntries; ent++) {
            uint64_t occ = populateMask(bishop_masks[sq], ent);
            auto index = (occ * bishop_magics[sq]) >> bishop_shifts[sq];
            bishop_attacks[sq][index] = generateBishopAttacks(sq, occ);
        }
    }
}

template <PColor Color>
void genCastling(SQ sq, brd::MoveList& mvList, const brd::BoardState& state) noexcept {
    constexpr uint64_t whiteShortCastl = 0x90;
    constexpr uint64_t whiteLongCastl = 0x11;
    constexpr uint64_t blackShortCastl= 0x9000000000000000;
    constexpr uint64_t blackLongCastl= 0x1100000000000000;

    constexpr uint64_t whiteShortCastlMask = 0xF0;
    constexpr uint64_t whiteLongCastlMask = 0x1F;
    constexpr uint64_t blackShortCastlMask = 0xF000000000000000;
    constexpr uint64_t blackLongCastlMask = 0x1F00000000000000;

    const BB occupied = state.getBoard().occupancy();
    auto&& board = state.getBoard();
    SG_ASSERT(board.getKind(1ull << sq) == PKind::pK);

    brd::CastlingType castling = brd::CastlingType::C_NONE;
    if constexpr (Color == PColor::W) {
        if (sq == makeSq(NFile::fE, NRank::r1)) {
            SG_ASSERT(Color == PColor::W);

            auto rooks = board.getPieceSqMask<Color, PKind::pR>();
            if((rooks & whiteShortCastl) && (whiteShortCastlMask & occupied) == whiteShortCastl
                && state.rightRookNotMoved<PColor::W>())
                castling = castling | brd::CastlingType::C_SHORT;
            if((rooks & whiteLongCastl) && (whiteLongCastlMask & occupied) == whiteLongCastl
                && state.leftRookNotMoved<PColor::W>())
                castling = castling | brd::CastlingType::C_LONG;
        }
    }
    else {
        if (sq == makeSq(NFile::fE, NRank::r8)) {
            SG_ASSERT(Color == PColor::B);

            auto rooks = board.getPieceSqMask<Color, PKind::pR>();
            if((rooks & blackShortCastl) && (blackShortCastlMask & occupied) == blackShortCastl
                && state.rightRookNotMoved<PColor::B>())
                castling = castling | brd::CastlingType::C_SHORT;
            if((rooks & blackLongCastl) && (blackLongCastlMask & occupied) == blackLongCastl
                && state.leftRookNotMoved<PColor::B>())
                castling = castling | brd::CastlingType::C_LONG;
        }
    }

    // state.kingUnderCheck<Color> check in the last order because of expencive attack map computation
    if (!castling || state.kingUnderCheck<Color>())
        return;

    if (castling & brd::CastlingType::C_SHORT)
        mvList.push(brd::mkCastling(sq, brd::CastlingType::C_SHORT));
    if (castling & brd::CastlingType::C_LONG)
        mvList.push(brd::mkCastling(sq, brd::CastlingType::C_LONG));
}


template void genCastling<PColor::W>(SQ sq, brd::MoveList& mvList, const brd::BoardState& state) noexcept;
template void genCastling<PColor::B>(SQ sq, brd::MoveList& mvList, const brd::BoardState& state) noexcept;
template uint64_t getPawnMoves<PColor::W>(SQ sq, const brd::BoardState&) noexcept;
template uint64_t getPawnMoves<PColor::B>(SQ sq, const brd::BoardState&) noexcept;
template uint64_t getPawnAttacks<PColor::W>(SQ sq) noexcept;
template uint64_t getPawnAttacks<PColor::B>(SQ sq) noexcept;
template uint8_t getEnpassantAttack<PColor::W>(SQ sq, const brd::BoardState& state) noexcept;
template uint8_t getEnpassantAttack<PColor::B>(SQ sq, const brd::BoardState& state) noexcept;

} // namespace movegen


