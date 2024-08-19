#include "defs.h"
#include <cstdlib>


/** Piece to char symbol */
char p2c(PKind kind, PColor color) noexcept {
    char r;
    switch(kind) {
        case PKind::pQ: r = 'Q'; break;
        case PKind::pB: r = 'B'; break;
        case PKind::pR: r = 'R'; break;
        case PKind::pK: r = 'K'; break;
        case PKind::pP: r = 'P'; break;
        case PKind::pN: r = 'N'; break;
        default: return '.';
    }
    return color == PColor::W ? r : static_cast<char>(r+0x20);
}

std::pair<PKind, PColor> c2p(char c) noexcept {
    switch (c) {
        case 'r': return std::make_pair(PKind::pR, PColor::B);
        case 'n': return std::make_pair(PKind::pN, PColor::B);
        case 'b': return std::make_pair(PKind::pB, PColor::B);
        case 'q': return std::make_pair(PKind::pQ, PColor::B);
        case 'k': return std::make_pair(PKind::pK, PColor::B);
        case 'p': return std::make_pair(PKind::pP, PColor::B);
        case 'R': return std::make_pair(PKind::pR, PColor::W);
        case 'N': return std::make_pair(PKind::pN, PColor::W);
        case 'B': return std::make_pair(PKind::pB, PColor::W);
        case 'Q': return std::make_pair(PKind::pQ, PColor::W);
        case 'K': return std::make_pair(PKind::pK, PColor::W);
        case 'P': return std::make_pair(PKind::pP, PColor::W);
        default: return std::make_pair(PKind::None, (PColor)false);
    }
}

uint8_t dist(SQ sq1, SQ sq2) noexcept {
    return std::abs((int)sq1 - (int)sq2);
}


