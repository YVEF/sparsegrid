#ifndef INCLUDE_TESTS_TEST_UTILS_H_
#define INCLUDE_TESTS_TEST_UTILS_H_

#include <board/board.h>

void preserveOnlyPositions(brd::Board& brd, std::initializer_list<uint8_t> positions) noexcept;



#define W_QUEEN_POS 3
#define B_QUEEN_POS 59

#define W_KING_POS 4
#define B_KING_POS 60

#define W_PAWN_1_POS 8
#define W_PAWN_2_POS 9
#define W_PAWN_3_POS 10
#define W_PAWN_4_POS 11
#define W_PAWN_5_POS 12
#define W_PAWN_6_POS 13
#define W_PAWN_7_POS 14
#define W_PAWN_8_POS 15

#define B_PAWN_1_POS 48
#define B_PAWN_2_POS 49
#define B_PAWN_3_POS 50
#define B_PAWN_4_POS 51
#define B_PAWN_5_POS 52
#define B_PAWN_6_POS 53
#define B_PAWN_7_POS 54
#define B_PAWN_8_POS 55

#define W_ROOK_1_POS 0
#define W_ROOK_2_POS 7
#define B_ROOK_1_POS 56
#define B_ROOK_2_POS 63

#define W_KNIGHT_1_POS 1
#define W_KNIGHT_2_POS 6
#define B_KNIGHT_1_POS 57
#define B_KNIGHT_2_POS 62

#define W_BISHOP_1_POS 2
#define W_BISHOP_2_POS 5
#define B_BISHOP_1_POS 58
#define B_BISHOP_2_POS 61






#endif  // INCLUDE_TESTS_TEST_UTILS_H_
