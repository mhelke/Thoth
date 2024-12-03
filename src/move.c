#include <stdio.h>

#include "move.h"

/*
Used to determine whether castling rights have changed.
These values are used to check if a move was made 
from or captured on square.
These values are created by taking the castling right & encoded move. 

No change:              15
White king moved:       12
White king rook moved:  14
White queen rook moved: 13    
Black king moved:       3
Black king rook moved:  11
Black queen rook moved: 7
 */
const int castling_rights[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

void generate_moves(Moves *moves) {
    moves->count = 0;
    generate_pawn_moves(side, moves);
    generate_knight_moves(side, moves);
    generate_bishop_moves(side, moves);
    generate_rook_moves(side, moves);
    generate_queen_moves(side, moves);
    generate_king_moves(side, moves);
    generate_castling_moves(side, moves);
}

void generate_pawn_moves(int side, Moves *moves) {
    int src, target, direction, opponent;
    int piece = (side == WHITE) ? P : p;
    Bitboard bitboard = bitboards[piece];
    direction = (side == WHITE) ? -8 : 8;
    opponent = 1 - side;
    int promotion_rank_start = (side == WHITE) ? a7 : a2;
    int promotion_rank_end = (side == WHITE) ? h7 : h2;
    int enpassant_rank = (side == WHITE) ? a5 : a4;
    int double_move_rank_start = (side == WHITE) ? a2 : a7;
    int double_move_rank_end = (side == WHITE) ? h2 : h7;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    const int *pieces;
    if (side == WHITE) {
        static const int white_pieces[] = {Q, R, B, N};
        pieces = white_pieces;
    } else {
        static const int black_pieces[] = {q, r, b, n};
        pieces = black_pieces;
    }


    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);
        target = src + direction; // pawn moves forward

        if ((side == WHITE && target >= a8) || (side == BLACK && target <= h1)) {
            if (!get_bit(occupancies[BOTH], target)) {
                // Promotion
                if (src >= promotion_rank_start && src <= promotion_rank_end) {
                    for (int i = 0; i < 4; ++i) {
                        int move = encode_move(src, target, piece, pieces[i], 0, 0, 0, 0);
                        add_move(moves, move);
                    }
                } else {
                    // One square
                    int move = encode_move(src, target, piece, 0, 0, 0, 0, 0);
                    add_move(moves, move);

                    // Pawn jump
                    if (src >= double_move_rank_start && src <= double_move_rank_end && !get_bit(occupancies[BOTH], target + direction)) {
                        move = encode_move(src, target + direction, piece, 0, 0, 1, 0, 0);
                        add_move(moves, move);
                    }
                }
            }
        }

        Bitboard attacks = pawn_attacks[side][src] & occupancies[opponent];
        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            // Promotion capture
            if (src >= promotion_rank_start && src <= promotion_rank_end) {
                for (int i = 0; i < 4; ++i) {
                    int move = encode_move(src, target, piece, pieces[i], 1, 0, 0, 0);
                    add_move(moves, move);
                }
            } else {
                // Regular capture
                int move = encode_move(src, target, piece, 0, 1, 0, 0, 0);
                add_move(moves, move);
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
                    int move = encode_move(src, ep_target, piece, 0, 1, 0, 1, 0);
                    add_move(moves, move);
                }
            }
        }
        pop_bit(bitboard, src);
    }
}

void generate_castling_moves(int side, Moves *moves) {
    const char *side_name = (side == WHITE) ? "White" : "Black";
    int piece = (side == WHITE)  ? K : k;
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
                int move = encode_move(src, k_target, piece, 0, 0, 0, 0, 1);
                add_move(moves, move);
            }
        }
    }

    // Queen-side
    if ((side == WHITE && (castle & WQ)) || (side == BLACK && (castle & BQ))) {
        if (!get_bit(occupancies[BOTH], q_pass) && !get_bit(occupancies[BOTH], q_target) && !get_bit(occupancies[BOTH], q_pass_second)) {
            if (!is_square_attacked(src, opponent) && !is_square_attacked(q_pass, opponent) && !is_square_attacked(q_target, opponent)) {
                int move = encode_move(src, q_target, piece, 0, 0, 0, 0, 1);
                add_move(moves, move);
            }
        }
    }
}

void generate_knight_moves(int side, Moves *moves) {
    int piece = (side == WHITE) ? N : n;
    int src, target;
    int opponent = 1 - side;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    Bitboard bitboard = bitboards[piece];

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);

        Bitboard attacks = knight_attacks[src] & (~occupancies[side]);

        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            if (get_bit(occupancies[opponent], target)) {
                // capture
                int move = encode_move(src, target, piece, 0, 1, 0, 0, 0);
                add_move(moves, move);
            } else {
                // normal
                int move = encode_move(src, target, piece, 0, 0, 0, 0, 0);
                add_move(moves, move);
            }

            pop_bit(attacks, target);
        }
        pop_bit(bitboard, src);
    }
}

void generate_bishop_moves(int side, Moves *moves) {
    int piece = (side == WHITE) ? B : b;
    int src, target;
    int opponent = 1 - side;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    Bitboard bitboard = bitboards[piece];

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);

        Bitboard attacks = get_bishop_attacks(src, occupancies[BOTH]) & (~occupancies[side]);

        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            if (get_bit(occupancies[opponent], target)) {
                // capture
                int move = encode_move(src, target, piece, 0, 1, 0, 0, 0);
                add_move(moves, move);
            } else {
                // normal
                int move = encode_move(src, target, piece, 0, 0, 0, 0, 0);
                add_move(moves, move);
            }

            pop_bit(attacks, target);
        }
        pop_bit(bitboard, src);
    }
}

void generate_rook_moves(int side, Moves *moves) {
    int piece = (side == WHITE) ? R : r;
    int src, target;
    int opponent = 1 - side;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    Bitboard bitboard = bitboards[piece];

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);

        Bitboard attacks = get_rook_attacks(src, occupancies[BOTH]) & (~occupancies[side]);

        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            if (get_bit(occupancies[opponent], target)) {
                // capture
                int move = encode_move(src, target, piece, 0, 1, 0, 0, 0);
                add_move(moves, move);
            } else {
                // normal
                int move = encode_move(src, target, piece, 0, 0, 0, 0, 0);
                add_move(moves, move);
            }

            pop_bit(attacks, target);
        }
        pop_bit(bitboard, src);
    }
}

void generate_queen_moves(int side, Moves *moves) {
    int piece = (side == WHITE) ? Q : q;
    int src, target;
    int opponent = 1 - side;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    Bitboard bitboard = bitboards[piece];

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);

        Bitboard attacks = get_queen_attacks(src, occupancies[BOTH]) & (~occupancies[side]);

        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            if (get_bit(occupancies[opponent], target)) {
                // capture
                int move = encode_move(src, target, piece, 0, 1, 0, 0, 0);
                add_move(moves, move);
            } else {
                // normal
                int move = encode_move(src, target, piece, 0, 0, 0, 0, 0);
                add_move(moves, move);
            }

            pop_bit(attacks, target);
        }
        pop_bit(bitboard, src);
    }
}

void generate_king_moves(int side, Moves *moves) {
    int piece = (side == WHITE) ? K : k;
    int src, target;
    int opponent = 1 - side;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    Bitboard bitboard = bitboards[piece];

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);

        Bitboard attacks = king_attacks[src] & (~occupancies[side]);

        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            if (get_bit(occupancies[opponent], target)) {
                // capture
                int move = encode_move(src, target, piece, 0, 1, 0, 0, 0);
                add_move(moves, move);
            } else {
                // normal
                int move = encode_move(src, target, piece, 0, 0, 0, 0, 0);
                add_move(moves, move);
            }

            pop_bit(attacks, target);
        }
        pop_bit(bitboard, src);
    }
}

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

void add_move(Moves *move_list, int move) {
    move_list->moves[move_list->count] = move;
    move_list->count++;
}

int make_move(int move, int move_type) {
    if (move_type == ALL_MOVES) {
        COPY_BOARD();

        int src = MOVE_SRC(move);
        int target = MOVE_TARGET(move);
        int piece = MOVE_PIECE(move);
        int promoted = MOVE_PROMOTED(move);
        int capture = MOVE_CAPTURE(move);
        int double_pawn = MOVE_DOUBLE(move);
        int ep = MOVE_ENPASSANT(move);
        int castled = MOVE_CASTLE(move);

        print_move(move);

        // Move piece from source to target
        pop_bit(bitboards[piece], src);
        set_bit(bitboards[piece], target);

        // Update occupancy tables
        pop_bit(occupancies[side], src);
        set_bit(occupancies[side], target);

        // Capture moves
        if (capture) {
            int captured_piece;
            int start = (side == WHITE) ? p : P;
            int end = (side == WHITE) ? k : K;

            for (captured_piece = start; captured_piece <= end; captured_piece++) {
                if (get_bit(bitboards[captured_piece], target)) {
                    pop_bit(bitboards[captured_piece], target);
                    break;
                }
            }
            pop_bit(occupancies[!side], target);
        }

        // Promotion Move
        if (promoted) {
            int pawn_bb = (side == WHITE) ? P : p;
            pop_bit(bitboards[pawn_bb], target);
            set_bit(bitboards[promoted], target);
            set_bit(occupancies[side], target);
        }

        // En passant
        if (ep) {
            int pawn_bb = (side == WHITE) ? p : P;
            int target_adj = (side == WHITE) ? 8 : -8;
            pop_bit(bitboards[pawn_bb], target + target_adj);
            pop_bit(occupancies[!side], target + target_adj);
        }

        // Reset En Passant Square
        enpassant = na;

        // Double Push - set en passant square
        if (double_pawn) {
            int target_adj = (side == WHITE) ? 8 : -8;
            enpassant = target + target_adj;
        }

        // Castling
        if (castled) {
            int rook_src[4] = {h1, a1, h8, a8};
            int rook_target[4] = {f1, d1, f8, d8};
            int rook_pieces[4] = {R, R, r, r};
            int targets[4] = {g1, c1, g8, c8};

            for (int i = 0; i < 4; i++) {
                if (target == targets[i]) {
                    pop_bit(bitboards[rook_pieces[i]], rook_src[i]);
                    set_bit(bitboards[rook_pieces[i]], rook_target[i]);
                    pop_bit(occupancies[side], rook_src[i]);
                    set_bit(occupancies[side], rook_target[i]);
                    break;
                }
            }
        }

        // Castling rights
        castle &= castling_rights[src];
        castle &= castling_rights[target];

        // Update overall occupancy table
        occupancies[BOTH] = occupancies[WHITE] | occupancies[BLACK];

        // Ensure King is not in Check
        side ^= 1; // change side
        if (is_square_attacked(get_least_sig_bit_index((side == WHITE) ? bitboards[k] : bitboards[K]), side)) {
            UNDO();
            return 0;
        }
        return 1;
    } else {
        // Only return capture moves
        int capture = MOVE_CAPTURE(move);
        if (!capture) {
            return 0;
        }
        make_move(move, ALL_MOVES);
    }
}

void print_move(int move) {
    int src = MOVE_SRC(move);
    int target = MOVE_TARGET(move);
    int promoted = MOVE_PROMOTED(move);

    printf("%s%s%c\n", square[src], square[target], promoted_pieces[promoted]);
}

void print_move_list(Moves *move_list) {
    if (!move_list->count) {
        printf("\nNo moves\n");
        return;
    }
    printf("\n\tPiece\tMove   Capture\tDouble\tEP\tCastling\n");
    for (int i = 0; i < move_list->count; i++) {
        int move = move_list->moves[i];
        int src = MOVE_SRC(move);
        int target = MOVE_TARGET(move);
        int promoted = MOVE_PROMOTED(move);
        int piece = MOVE_PIECE(move);
        int capture = MOVE_CAPTURE(move);
        int d_push = MOVE_DOUBLE(move);
        int ep = MOVE_ENPASSANT(move);
        int castling = MOVE_CASTLE(move);
    printf("%d.\t%c\t%s%s%c\t%d\t%d\t%d\t%d\n", i+1, ascii_pieces[piece], square[src], square[target], promoted ? promoted_pieces[promoted] : ' ', capture, d_push, ep, castling);
    }
}