#include <stdio.h>

#include "move.h"
#include "gen.h"

void generate_moves() {
    side ? generate_black_moves() : generate_white_moves();
}

void generate_white_moves() {
    generate_pawn_moves(WHITE);
    generate_castling_moves(WHITE);
    generate_knight_moves(WHITE);

}

void generate_black_moves() {
    generate_pawn_moves(BLACK);
    generate_castling_moves(BLACK);
    generate_knight_moves(BLACK);
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
                // Regular capture
                printf("%s Pawn Capture: %sx%s\n", side_name, square[src], square[target]);
            }

            pop_bit(attacks, target);
        }

        // En Passant
        if (enpassant != na) {
            int enpassant_file = enpassant % 8;
            if (src >= enpassant_rank && src <= enpassant_rank + 7 &&
                (src % 8 == enpassant_file - 1 || src % 8 == enpassant_file + 1)) {
                Bitboard enpassant_attacks = pawn_attacks[side][src] & (1ULL << enpassant);
                if (enpassant_attacks) {
                    int ep_target = get_least_sig_bit_index(enpassant_attacks);
                    printf("%s En Passant Capture: %sx%sep\n", side_name, square[src], square[ep_target]);
                }
            }
        }
        pop_bit(bitboard, src);
    }
}

void generate_castling_moves(int side) {
    const char *side_name = (side == WHITE) ? "White" : "Black";
    int src = (side == WHITE) ? e1 : e8;
    int k_target = (side == WHITE) ? g1 : g8;
    int q_target = (side == WHITE) ? c1 : c8;
    int q_pass = (side == WHITE) ? d1 : d8;
    int q_pass_second = (side == WHITE) ? b1 : b8;
    int k_pass = (side == WHITE) ? f1 : f8;
    int opponent = 1 - side;

    // King-side
    if ((side == WHITE && (castle & WK)) || (side == BLACK && (castle & BK))) {
        if (!get_bit(occupancies[BOTH], k_pass) && !get_bit(occupancies[BOTH], k_target)) {
            if (!is_square_attacked(src, opponent) && !is_square_attacked(k_pass, opponent) && !is_square_attacked(k_target, opponent)) {
                printf("%s O-O: %s %s\n", side_name, square[src], square[k_target]);
            }
        }
    }

    // Queen-side
    if ((side == WHITE && (castle & WQ)) || (side == BLACK && (castle & BQ))) {
        if (!get_bit(occupancies[BOTH], q_pass) && !get_bit(occupancies[BOTH], q_target) && !get_bit(occupancies[BOTH], q_pass_second)) {
            if (!is_square_attacked(src, opponent) && !is_square_attacked(q_pass, opponent) && !is_square_attacked(q_target, opponent)) {
                printf("%s O-O-O: %s %s\n", side_name, square[src], square[q_target]);
            }
        }
    }
}

void generate_knight_moves(int side) {
    int src, target;
    int opponent = 1 - side;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    Bitboard bitboard = (side == WHITE) ? bitboards[N] : bitboards[n];

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);

        Bitboard attacks = knight_attacks[src] & (~occupancies[side]);

        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            if (get_bit(occupancies[opponent], target)) {
                // capture
                printf("%s N capture: %sx%s\n", side_name, square[src], square[target]);
            } else {
                // normal
                printf("%s N: %s %s\n", side_name, square[src], square[target]);
            }

            pop_bit(attacks, target);
        }
        pop_bit(bitboard, src);
    }
}



