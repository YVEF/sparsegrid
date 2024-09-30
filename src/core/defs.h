#ifndef INCLUDE_CORE_DEFS_H_
#define INCLUDE_CORE_DEFS_H_
#include <bit>
#include <cstdint>
#include <utility>

typedef uint8_t SQ;
constexpr SQ SQ_CNT = 64;
typedef uint64_t BB;


typedef uint64_t QMoves;
typedef uint64_t AMoves;

typedef int16_t Score;


// Piece Kind
enum PKind : uint8_t {
    None = 0,
    pP,
    pK,
    pQ,
    pB,
    pN,
    pR
};


enum PColor : bool {
    W = true,
    B = false
};


enum NRank : uint64_t {
    r1 = 0xFFULL,
    r2 = r1 << 0x08,
    r3 = r2 << 0x08,
    r4 = r3 << 0x08,
    r5 = r4 << 0x08,
    r6 = r5 << 0x08,
    r7 = r6 << 0x08,
    r8 = r7 << 0x08
};

enum NFile : uint64_t {
    fA = 0x101010101010101ULL,
    fB = fA << 1,
    fC = fB << 1,
    fD = fC << 1,
    fE = fD << 1,
    fF = fE << 1,
    fG = fF << 1,
    fH = fG << 1
};



enum Square {
    sq_a1 = NFile::fA & (uint64_t)NRank::r1,
    sq_a2 = NFile::fA & (uint64_t)NRank::r2,
    sq_a3 = NFile::fA & (uint64_t)NRank::r3,
    sq_a4 = NFile::fA & (uint64_t)NRank::r4,

    sq_b3 = NFile::fB & (uint64_t)NRank::r3,
    sq_b5 = NFile::fB & (uint64_t)NRank::r5,

    sq_c2 = NFile::fC & (uint64_t)NRank::r2,
    sq_c5 = NFile::fC & (uint64_t)NRank::r5,

    sq_d5 = NFile::fD & (uint64_t)NRank::r5,
    sq_d6 = NFile::fD & (uint64_t)NRank::r6,

    sq_e2 = NFile::fE & (uint64_t)NRank::r2,
    sq_e5 = NFile::fE & (uint64_t)NRank::r5,
    sq_e7 = NFile::fE & (uint64_t)NRank::r7,
    sq_e8 = NFile::fE & (uint64_t)NRank::r8,

    sq_g1 = NFile::fG & (uint64_t)NRank::r1,
    sq_g3 = NFile::fG & (uint64_t)NRank::r3,
    sq_g4 = NFile::fG & (uint64_t)NRank::r4,

    sq_f1 = NFile::fF & (uint64_t)NRank::r1,
    sq_f3 = NFile::fF & (uint64_t)NRank::r3,
    sq_f4 = NFile::fF & (uint64_t)NRank::r4,
    sq_f5 = NFile::fF & (uint64_t)NRank::r5,
    sq_f6 = NFile::fF & (uint64_t)NRank::r6,

    sq_h5 = NFile::fH & (uint64_t)NRank::r5,
};


enum SqNum {
    sqn_a1 = 0,
    sqn_a2 = 8,
    sqn_a3 = 16,
    sqn_a4 = 24,
    sqn_a5 = 32,
    sqn_a6 = 40,
    sqn_a7 = 48,
    sqn_a8 = 56,

    sqn_b2 = 9,
    sqn_b3 = 17,
    sqn_b4 = 25,
    sqn_b5 = 33,
    sqn_b6 = 41,
    sqn_b7 = 49,

    sqn_c1 = 2,
    sqn_c2 = 10,
    sqn_c3 = 18,
    sqn_c4 = 26,
    sqn_c5 = 34,
    sqn_c7 = 50,
    sqn_c8 = 58,

    sqn_d1 = 3,
    sqn_d2 = 11,
    sqn_d3 = 19,
    sqn_d4 = 27,
    sqn_d5 = 35,
    sqn_d6 = 43,
    sqn_d7 = 51,
    sqn_d8 = 59,

    sqn_f1 = 5,
    sqn_f2 = 13,
    sqn_f3 = 21,
    sqn_f5 = 37,
    sqn_f6 = 45,
    sqn_f7 = 53,
    sqn_f8 = 61,

    sqn_g1 = 6,
    sqn_g2 = 14,
    sqn_g5 = 38,
    sqn_g8 = 62,


    sqn_e1 = 4,
    sqn_e2 = 12,
    sqn_e3 = 20,
    sqn_e4 = 28,
    sqn_e5 = 36,
    sqn_e6 = 44,
    sqn_e7 = 52,
    sqn_e8 = 60,


    sqn_h1 = 7,
    sqn_h2 = 15,
    sqn_h3 = 23,
    sqn_h4 = 31,
    sqn_h5 = 39,
    sqn_h6 = 47,
    sqn_h7 = 55,
    sqn_h8 = 63,

};

constexpr uint64_t OUTER_SQUARES_BB = 0xFF818181818181FFULL;



#define TEMPLATE_DEF(rett, method, ...) TEMPLATE_DEF_CORE(rett, method, noexcept, , __VA_ARGS__)
#define TEMPLATE_DEF_CONST(rett, method, ...) TEMPLATE_DEF_CORE(rett, method, noexcept, const , __VA_ARGS__)

#define TEMPLATE_DEF_CORE(rett, method, noex, cons, ...) \
template rett method<PColor::W, PKind::pQ>(__VA_ARGS__) cons noex; \
template rett method<PColor::W, PKind::pK>(__VA_ARGS__) cons noex; \
template rett method<PColor::W, PKind::pB>(__VA_ARGS__) cons noex; \
template rett method<PColor::W, PKind::pR>(__VA_ARGS__) cons noex; \
template rett method<PColor::W, PKind::pN>(__VA_ARGS__) cons noex; \
template rett method<PColor::W, PKind::pP>(__VA_ARGS__) cons noex; \
template rett method<PColor::B, PKind::pQ>(__VA_ARGS__) cons noex; \
template rett method<PColor::B, PKind::pK>(__VA_ARGS__) cons noex; \
template rett method<PColor::B, PKind::pB>(__VA_ARGS__) cons noex; \
template rett method<PColor::B, PKind::pR>(__VA_ARGS__) cons noex; \
template rett method<PColor::B, PKind::pN>(__VA_ARGS__) cons noex; \
template rett method<PColor::B, PKind::pP>(__VA_ARGS__) cons noex; 



inline PColor invert(PColor color) noexcept {
    return static_cast<PColor>(!((bool)color));
}

inline constexpr unsigned int col2int(PColor color) noexcept {
    return static_cast<unsigned int>(color);
}

constexpr inline SQ popLsb(BB& mask) noexcept {
    SQ r = std::countr_zero(mask);
    mask &= (mask - 1);
    return r;
}

uint8_t dist(SQ sq1, SQ sq2) noexcept;

/** Get char representation of the piece */
char p2c(PKind kind, PColor color) noexcept;
/** Get piece by the char */
std::pair<PKind, PColor> c2p(char) noexcept;
constexpr inline SQ makeSq(NFile file, NRank rank) noexcept {
    BB mask = (BB)file & (BB)rank;
    return (SQ)popLsb(mask);
}


#define FEN_SHORT_WHITE_CASTLE_MASK 0x01
#define FEN_LONG_WHITE_CASTLE_MASK 0x02
#define FEN_SHORT_BLACK_CASTLE_MASK 0x04
#define FEN_LONG_BLACK_CASTLE_MASK 0x08

#define CASTL_TO_UCI_CASTL(castl, from) ((castl) == brd::CastlingType::C_SHORT ? (from) + 3 : (from) - 4)
#define AS_BB(sq) (1ull << (sq))

#endif  // INCLUDE_CORE_DEFS_H_
