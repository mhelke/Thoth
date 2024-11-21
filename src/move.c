#include "move.h"
#include <stdio.h>

void generate_moves() {
    side ? generate_black_moves() : generate_white_moves();
}

void generate_white_moves() {
    generate_pawn_moves(WHITE);
}

void generate_black_moves() {
    generate_pawn_moves(BLACK);
}

void generate_pawn_moves(int side) {
    int src, target, direction, opponent;
    Bitboard bitboard = (side == WHITE) ? bitboards[P] : bitboards[p];
    direction = (side == WHITE) ? -8 : 8;
    opponent = 1 - side;
    int promotion_rank_start = (side == WHITE) ? a7 : a2;
    int promotion_rank_end = (side == WHITE) ? h7 : h2;
    int enpassant_rank = (side == WHITE) ? a5 : a4;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);
        target = src + direction; // pawn moves forward

        if ((side == WHITE && target >= a8) || (side == BLACK && target <= h1)) {
            if (!get_bit(occupancies[BOTH], target)) {
                // Promotion
                if (src >= promotion_rank_start && src <= promotion_rank_end) {
                    const char *pieces[] = {"q", "r", "b", "n"};
                    for (int i = 0; i < 4; ++i) {
                        printf("%s Pawn Promotion: %s %s=%s\n", side_name, square[src], square[target], pieces[i]);
                    }
                } else {
                    // One square
                    printf("%s Pawn Move: %s %s\n", side_name, square[src], square[target]);

                    // Pawn jump
                    if ((src >= a2 && src <= h2) && !get_bit(occupancies[BOTH], target + direction)) {
                        printf("%s Pawn Jump: %s %s\n", side_name, square[src], square[target + direction]);
                    }
                }
            }
        }

        Bitboard attacks = pawn_attacks[side][src] & occupancies[opponent];
        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            // Promotion capture
            if (src >= promotion_rank_start && src <= promotion_rank_end) {
                const char *pieces[] = {"q", "r", "b", "n"};
                for (int i = 0; i < 4; ++i) {
                    printf("%s Pawn Promotion Capture: %sx%s=%s\n", side_name, square[src], square[target], pieces[i]);
                }
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
