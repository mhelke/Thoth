#include <stdio.h>

#include "bitboard.h"

// print bitboard
void print_bitboard(U64 bitboard) {

    printf("\n");

    // loop over ranks
    for (int rank = 0; rank < 8; rank++) {
        // loop over files
        for (int file = 0; file < 8; file++) {
            // convert file & rank into square
            int square = rank * 8 + file;

            // print ranks
            if (!file) {
                printf(" %d ", 8 - rank);
            }

            // bits state - 1 or 0
            printf(" %d ", get_bit(bitboard, square) ? 1 : 0);
        }
        printf("\n");
    }

    // print files
    printf("\n    a  b  c  d  e  f  g  h  \n\n");

    // print bitboard as unsigned decimal
    printf("Bitboard: %llud\n\n", bitboard);
}