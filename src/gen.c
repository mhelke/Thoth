// #include "attack_tables.h"
// #include "bitboard.h"

/***
    If the resulting output is a chessboard, the number at each index is the number of relevant occupancy squares within the path of the bishop's attack mask (moves).
    e.g. on a8, the bishop has a total of 6 relevant occupancy bits (down the long diagonal, not counting h1) 
    e.g. In the center of the board, the bishop has 9 total relevant occupancy squares. 

    Output:
        6, 5, 5, 5, 5, 5, 5, 6,
        5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 9, 9, 7, 5, 5,
        5, 5, 7, 7, 7, 7, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5,
        6, 5, 5, 5, 5, 5, 5, 6,
***/
/*
void gen_bishop_relevant_bits() {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            printf(" %d,", count_bits(mask_bishop_attacks(square)));
        }
        printf("\n");
    }
};
*/


/***
    If the resulting output is a chessboard, the number at each index is the number of relevant occupancy squares within the path of the rooks's attack mask (moves).
    e.g. on a8, the rook has a total of 12 relevant occupancy bits (down the rank and file, not counting a1 or h8) 
    e.g. In the center of the board, the rook has 10 total relevant occupancy squares. 

    Output:
        12, 11, 11, 11, 11, 11, 11, 12,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        11, 10, 10, 10, 10, 10, 10, 11,
        12, 11, 11, 11, 11, 11, 11, 12,
       
***/
/*
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;
            printf(" %d,", count_bits(mask_rook_attacks(square)));
        }
        printf("\n");
    }
*/