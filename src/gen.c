#include <stdio.h>
#include "gen.h"

int is_square_attacked(int square, int side) {
    int opponent = (side == WHITE) ? BLACK : WHITE;
    int offset = (side == WHITE) ? 0 : 6;
    
    if (pawn_attacks[opponent][square] & bitboards[P + offset]) return 1;
    if (knight_attacks[square] & bitboards[N + offset]) return 1;
    if (get_bishop_attacks(square, occupancies[BOTH]) & bitboards[B + offset]) return 1;
    if (get_rook_attacks(square, occupancies[BOTH]) & bitboards[R + offset]) return 1;    
    if (get_queen_attacks(square, occupancies[BOTH]) & bitboards[Q + offset]) return 1;
    if (king_attacks[square] & bitboards[K + offset]) return 1;
    
    return 0;
}

void print_attacked_squares(int side) {
    printf("\n");
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;       
            if (!file) {
                printf("%d ", 8 - rank);
            }
            printf("  %d", is_square_attacked(square, side) ? 1 : 0);
        }        
        printf("\n");
    }
    printf("\n    a  b  c  d  e  f  g  h\n\n");
}