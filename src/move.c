#include "move.h"
#include <stdio.h>

void generate_moves() {
    generate_white_moves();
    generate_black_moves();
}

void generate_white_moves() {
    generate_white_pawn_moves();
  

}

void generate_black_moves() {
    generate_black_pawn_moves();
}

void generate_white_pawn_moves() {
    int src, target;
    Bitboard bitboard = bitboards[P];
    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);
        target = src - 8; // white pawn moves forward


        if ((target >= a8) && !get_bit(occupancies[BOTH], target)) {
            // Promotion
            if (src >= a7 && src <= h7) {
                printf("White Pawn Promotion: %s %s=q\n", square[src], square[target]);
                printf("White Pawn Promotion: %s %s=r\n", square[src], square[target]);
                printf("White Pawn Promotion: %s %s=b\n", square[src], square[target]);
                printf("White Pawn Promotion: %s %s=n\n", square[src], square[target]);
            } else {
                // one square
                printf("White Pawn Move: %s %s\n", square[src], square[target]);

                // pawn jump
                if ((src >= a2 && src <= h2) && !get_bit(occupancies[BOTH], target - 8)) {
                    printf("White Pawn Jump: %s %s\n", square[src], square[target - 8]);
                }
            }
        } else {
            printf("Cannot Move: %s\n", square[src]);
        }
        pop_bit(bitboard, src);
    }
}

void generate_black_pawn_moves() {
    int src, target;
    Bitboard bitboard = bitboards[p];
    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);
        target = src + 8; // black pawn moves forward


        if ((target <= h1) && !get_bit(occupancies[BOTH], target)) {
            // Promotion
            if (src >= a2 && src <= h2) {
                printf("Black Pawn Promotion: %s %s=q\n", square[src], square[target]);
                printf("Black Pawn Promotion: %s %s=r\n", square[src], square[target]);
                printf("Black Pawn Promotion: %s %s=b\n", square[src], square[target]);
                printf("Black Pawn Promotion: %s %s=n\n", square[src], square[target]);
            } else {
                // one square
                printf("Black Pawn Move: %s %s\n", square[src], square[target]);

                // pawn jump
                if ((src >= a7 && src <= h7) && !get_bit(occupancies[BOTH], target + 8)) {
                    printf("Black Pawn Jump: %s %s\n", square[src], square[target + 8]);
                }
            }
        } else {
            printf("Cannot Move: %s\n", square[src]);
        }
        pop_bit(bitboard, src);
    }
}
