#include "fen.h"
#include "../core/defs.h"
#include "../board/board_state.h"
#include <sstream>


namespace uci {


std::size_t Fen::apply(std::string_view input, brd::BoardState& state) {
    SQ sq = SqNum::sqn_a8;
    auto& board = state.getBoardMutable();
    board.clear();
    state.FenResetState();

    std::size_t i=0;
    while(input[i] == ' ') i++;

    for(; input[i] != ' '; i++) {
        char c = input[i];
        if(c == '/') {
            sq -= 16;
            continue;
        }
        if(std::isdigit(c)) {
            int empty_sq = c - '0';
            sq += empty_sq;
            continue;
        }

        auto [kind, col] = c2p(c);
        board.put(kind, col, sq++);
    }

    PColor tomove;
    if(input[++i] == 'w') tomove = PColor::W;
    else tomove = PColor::B;
    state.FenSetNextPlayer(tomove);

    i+=2;
    // available castling
    while(input[i] != ' ') {
        switch(input[i++]) {
            case 'K': ; state.setFenCastlingMask(FEN_SHORT_WHITE_CASTLE_MASK); break;
            case 'Q': ; state.setFenCastlingMask(FEN_LONG_WHITE_CASTLE_MASK); break;
            case 'k': ; state.setFenCastlingMask(FEN_SHORT_BLACK_CASTLE_MASK); break;
            case 'q': ; state.setFenCastlingMask(FEN_LONG_BLACK_CASTLE_MASK); break;
            default: continue;
        }
    }
    i++;

    // available enpassant
    if(input[i] == '-') {
        i++;
    }
    else {
         auto fl = input[i] - 'a';
         auto rnk = input[i+1] - '1';
         state.FenSetEnpass(fl + rnk*8);
         i+=2;
    }
    while(input[i] != ' ') i++;
    i++;

    // check rule 50 ply clock
    int ply = 0;
    while(input[i] != ' ') {
        ply *= 10;
        ply += (input[i++] - '0');
    }
    state.resetState(ply);
    i++;

    // skip the total full moves and point to the end
    while(i < input.size() && input[i] != ' ') i++;
    while(i < input.size() && input[i] == ' ') i++;

    state.markBuildFromFen();
    return i;
}

std::string Fen::str(const brd::BoardState& state) const noexcept {
    std::stringstream ss;
    auto&& board = state.getBoard();
    int empty = 0;
    for (int i=(int)SqNum::sqn_a8; i>=0; i++) {
        for (empty = 0; board.empty(i); i++) {
            empty++;
            if (i % 8 == 7) break;
        }

        if (empty) ss << empty;
        if (i % 8 == 7) {
            i -= 16;
            if (i >= -1) ss << '/';
            continue;
        }
        else {
            BB mask = 1ull << i;
            ss << p2c(board.getKind(mask), board.getColor(mask));
        }
    }

    ss << (state.ply() % 2 ? " b " : " w ");
    ss << "...";

    return ss.str();
}


} // namespace uci
