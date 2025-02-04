#include <stdio.h>

#include "move.h"
#include "table.h"
#include "board.h"

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

void generate_moves(Moves *moves, Board *board) {
    moves->count = 0;
    generate_pawn_moves(board->side, moves, board);
    generate_knight_moves(board->side, moves, board);
    generate_bishop_moves(board->side, moves, board);
    generate_rook_moves(board->side, moves, board);
    generate_queen_moves(board->side, moves, board);
    generate_king_moves(board->side, moves, board);
    generate_castling_moves(board->side, moves, board);
}

void generate_pawn_moves(int side, Moves *moves, Board *board) {
    int src, target, direction, opponent;
    int piece = (side == WHITE) ? P : p;
    Bitboard bitboard = board->bitboards[piece];
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
            if (!GET_BIT(board->occupancies[BOTH], target)) {
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
                    if (src >= double_move_rank_start && src <= double_move_rank_end && !GET_BIT(board->occupancies[BOTH], target + direction)) {
                        move = encode_move(src, target + direction, piece, 0, 0, 1, 0, 0);
                        add_move(moves, move);
                    }
                }
            }
        }

        Bitboard attacks = board->pawn_attacks[side][src] & board->occupancies[opponent];
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

            POP_BIT(attacks, target);
        }

        // En Passant
        if (board->enpassant != na) {
            int enpassant_file = board->enpassant % 8;
            if (src >= enpassant_rank && src <= enpassant_rank + 7 &&
                (src % 8 == enpassant_file - 1 || src % 8 == enpassant_file + 1)) {
                Bitboard enpassant_attacks = board->pawn_attacks[side][src] & (1ULL << board->enpassant);
                if (enpassant_attacks) {
                    int ep_target = get_least_sig_bit_index(enpassant_attacks);
                    int move = encode_move(src, ep_target, piece, 0, 1, 0, 1, 0);
                    add_move(moves, move);
                }
            }
        }
        POP_BIT(bitboard, src);
    }
}

void generate_castling_moves(int side, Moves *moves, Board *board) {
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
    if ((side == WHITE && (board->castle & WK)) || (side == BLACK && (board->castle & BK))) {
        if (!GET_BIT(board->occupancies[BOTH], k_pass) && !GET_BIT(board->occupancies[BOTH], k_target)) {
            if (!is_square_attacked(src, opponent, board) && !is_square_attacked(k_pass, opponent, board) && !is_square_attacked(k_target, opponent, board)) {
                int move = encode_move(src, k_target, piece, 0, 0, 0, 0, 1);
                add_move(moves, move);
            }
        }
    }

    // Queen-side
    if ((side == WHITE && (board->castle & WQ)) || (side == BLACK && (board->castle & BQ))) {
        if (!GET_BIT(board->occupancies[BOTH], q_pass) && !GET_BIT(board->occupancies[BOTH], q_target) && !GET_BIT(board->occupancies[BOTH], q_pass_second)) {
            if (!is_square_attacked(src, opponent, board) && !is_square_attacked(q_pass, opponent, board) && !is_square_attacked(q_target, opponent, board)) {
                int move = encode_move(src, q_target, piece, 0, 0, 0, 0, 1);
                add_move(moves, move);
            }
        }
    }
}

void generate_knight_moves(int side, Moves *moves, Board *board) {
    int piece = (side == WHITE) ? N : n;
    int src, target;
    int opponent = 1 - side;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    Bitboard bitboard = board->bitboards[piece];

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);

        Bitboard attacks = board->knight_attacks[src] & (~board->occupancies[side]);

        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            if (GET_BIT(board->occupancies[opponent], target)) {
                // capture
                int move = encode_move(src, target, piece, 0, 1, 0, 0, 0);
                add_move(moves, move);
            } else {
                // normal
                int move = encode_move(src, target, piece, 0, 0, 0, 0, 0);
                add_move(moves, move);
            }

            POP_BIT(attacks, target);
        }
        POP_BIT(bitboard, src);
    }
}

void generate_bishop_moves(int side, Moves *moves, Board *board) {
    int piece = (side == WHITE) ? B : b;
    int src, target;
    int opponent = 1 - side;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    Bitboard bitboard = board->bitboards[piece];

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);

        Bitboard attacks = get_bishop_attacks(src, board->occupancies[BOTH], board) & (~board->occupancies[side]);

        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            if (GET_BIT(board->occupancies[opponent], target)) {
                // capture
                int move = encode_move(src, target, piece, 0, 1, 0, 0, 0);
                add_move(moves, move);
            } else {
                // normal
                int move = encode_move(src, target, piece, 0, 0, 0, 0, 0);
                add_move(moves, move);
            }

            POP_BIT(attacks, target);
        }
        POP_BIT(bitboard, src);
    }
}

void generate_rook_moves(int side, Moves *moves, Board *board) {
    int piece = (side == WHITE) ? R : r;
    int src, target;
    int opponent = 1 - side;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    Bitboard bitboard = board->bitboards[piece];

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);

        Bitboard attacks = get_rook_attacks(src, board->occupancies[BOTH], board) & (~board->occupancies[side]);

        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            if (GET_BIT(board->occupancies[opponent], target)) {
                // capture
                int move = encode_move(src, target, piece, 0, 1, 0, 0, 0);
                add_move(moves, move);
            } else {
                // normal
                int move = encode_move(src, target, piece, 0, 0, 0, 0, 0);
                add_move(moves, move);
            }

            POP_BIT(attacks, target);
        }
        POP_BIT(bitboard, src);
    }
}

void generate_queen_moves(int side, Moves *moves, Board *board) {
    int piece = (side == WHITE) ? Q : q;
    int src, target;
    int opponent = 1 - side;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    Bitboard bitboard = board->bitboards[piece];

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);

        Bitboard attacks = get_queen_attacks(src, board->occupancies[BOTH], board) & (~board->occupancies[side]);

        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            if (GET_BIT(board->occupancies[opponent], target)) {
                // capture
                int move = encode_move(src, target, piece, 0, 1, 0, 0, 0);
                add_move(moves, move);
            } else {
                // normal
                int move = encode_move(src, target, piece, 0, 0, 0, 0, 0);
                add_move(moves, move);
            }

            POP_BIT(attacks, target);
        }
        POP_BIT(bitboard, src);
    }
}

void generate_king_moves(int side, Moves *moves, Board *board) {
    int piece = (side == WHITE) ? K : k;
    int src, target;
    int opponent = 1 - side;
    const char *side_name = (side == WHITE) ? "White" : "Black";

    Bitboard bitboard = board->bitboards[piece];

    while (bitboard) {
        src = get_least_sig_bit_index(bitboard);

        Bitboard attacks = board->king_attacks[src] & (~board->occupancies[side]);

        while (attacks) {
            target = get_least_sig_bit_index(attacks);

            if (GET_BIT(board->occupancies[opponent], target)) {
                // capture
                int move = encode_move(src, target, piece, 0, 1, 0, 0, 0);
                add_move(moves, move);
            } else {
                // normal
                int move = encode_move(src, target, piece, 0, 0, 0, 0, 0);
                add_move(moves, move);
            }

            POP_BIT(attacks, target);
        }
        POP_BIT(bitboard, src);
    }
}

int is_square_attacked(int square, int side, Board *board) {
    int opponent = (side == WHITE) ? BLACK : WHITE;
    int offset = (side == WHITE) ? 0 : 6;
    
    if (board->pawn_attacks[opponent][square] & board->bitboards[P + offset]) return 1;
    if (board->knight_attacks[square] & board->bitboards[N + offset]) return 1;
    if (get_bishop_attacks(square, board->occupancies[BOTH], board) & board->bitboards[B + offset]) return 1;
    if (get_rook_attacks(square, board->occupancies[BOTH], board) & board->bitboards[R + offset]) return 1;    
    if (get_queen_attacks(square, board->occupancies[BOTH], board) & board->bitboards[Q + offset]) return 1;
    if (board->king_attacks[square] & board->bitboards[K + offset]) return 1;
    
    return 0;
}

Bitboard get_attackers_to_square(int target_square, int side, Bitboard occupancy[], Bitboard bitboards[], Board *board) {
    Bitboard attackers = 0ULL;
    int offset = (side == WHITE) ? 0 : 6;

    // Get attackers for the provided side to the target square
    attackers |= board->pawn_attacks[side][target_square] & occupancy[side^1];
    attackers |= board->knight_attacks[target_square] & bitboards[N + offset];
    attackers |= get_bishop_attacks(target_square, occupancy[BOTH], board) & bitboards[B + offset];
    attackers |= get_rook_attacks(target_square, occupancy[BOTH], board) & bitboards[R + offset];   
    attackers |= get_queen_attacks(target_square, occupancy[BOTH], board) & bitboards[Q + offset];
    attackers |= board->king_attacks[target_square] & bitboards[K + offset];

    return attackers;
}

void add_move(Moves *move_list, int move) {
    move_list->moves[move_list->count] = move;
    move_list->count++;
}

int make_move(int move, Board *board) {
    COPY_BOARD(board);

    int src = MOVE_SRC(move);
    int target = MOVE_TARGET(move);
    int piece = MOVE_PIECE(move);

    // Move piece from source to target
    POP_BIT(board->bitboards[piece], src);
    SET_BIT(board->bitboards[piece], target);

    // Update occupancy tables
    POP_BIT(board->occupancies[board->side], src);
    SET_BIT(board->occupancies[board->side], target);

    board->hash_key ^= piece_keys[piece][src];
    board->hash_key ^= piece_keys[piece][target];

    // Increment 50 move rule counter.
    board->fifty_move_rule_counter++;

    // Pawn moves reset the 50 move rule.
    if (piece == P || piece == p) {
        board->fifty_move_rule_counter = 0;
    }

    // Capture moves
    if (MOVE_CAPTURE(move)) {
        // Captures reset the 50 move rule.
        board->fifty_move_rule_counter = 0;
        int captured_piece;
        int start = (board->side == WHITE) ? p : P;
        int end = (board->side == WHITE) ? k : K;

        for (captured_piece = start; captured_piece <= end; captured_piece++) {
            if (GET_BIT(board->bitboards[captured_piece], target)) {
                POP_BIT(board->bitboards[captured_piece], target);
                board->hash_key ^= piece_keys[captured_piece][target];
                break;
            }
        }
        POP_BIT(board->occupancies[!board->side], target);
    }

    // Promotion Move
    if (MOVE_PROMOTED(move)) {
        int pawn_bb = (board->side == WHITE) ? P : p;
        POP_BIT(board->bitboards[pawn_bb], target);
        SET_BIT(board->bitboards[MOVE_PROMOTED(move)], target);
        SET_BIT(board->occupancies[board->side], target);
        board->hash_key ^= piece_keys[pawn_bb][target];
        board->hash_key ^= piece_keys[MOVE_PROMOTED(move)][target];
    }

    // En passant
    if (MOVE_ENPASSANT(move)) {
        int pawn_bb = (board->side == WHITE) ? p : P;
        int target_adj = (board->side == WHITE) ? 8 : -8;
        POP_BIT(board->bitboards[pawn_bb], target + target_adj);
        POP_BIT(board->occupancies[!board->side], target + target_adj);

        board->hash_key ^= piece_keys[pawn_bb][(target + target_adj)];
    }

    if (board->enpassant != na) {
        board->hash_key ^= enpassant_keys[board->enpassant];
    }

    // Reset En Passant Square
    board->enpassant = na;

    // Double Push - set en passant square
    if (MOVE_DOUBLE(move)) {
        board->enpassant = target + ((board->side == WHITE) ? 8 : -8);
        board->hash_key ^= enpassant_keys[target + ((board->side == WHITE) ? 8 : -8)];
    }

    // Castling
    if (MOVE_CASTLE(move)) {
        int rook_src[4] = {h1, a1, h8, a8};
        int rook_target[4] = {f1, d1, f8, d8};
        int rook_pieces[4] = {R, R, r, r};
        int targets[4] = {g1, c1, g8, c8};

        for (int i = 0; i < 4; i++) {
            if (target == targets[i]) {
                POP_BIT(board->bitboards[rook_pieces[i]], rook_src[i]);
                SET_BIT(board->bitboards[rook_pieces[i]], rook_target[i]);
                POP_BIT(board->occupancies[board->side], rook_src[i]);
                SET_BIT(board->occupancies[board->side], rook_target[i]);

                board->hash_key ^= piece_keys[rook_pieces[i]][rook_src[i]];
                board->hash_key ^= piece_keys[rook_pieces[i]][rook_target[i]];

                break;
            }
        }
    }

    board->hash_key ^= castling_keys[board->castle];

    // Castling rights
    board->castle &= castling_rights[src];
    board->castle &= castling_rights[target];

    board->hash_key ^= castling_keys[board->castle];

    // Update overall occupancy table
    board->occupancies[BOTH] = board->occupancies[WHITE] | board->occupancies[BLACK];

    // Change side
    board->side ^= 1;

    board->hash_key ^= side_key;

    // Debug hash key generation
    /*
    Bitboard expected_hash = generate_hash_key(board);
    if (board->hash_key != expected_hash) {
        printf("Within make_move...\n");
        printf("Move: ");
        print_move(move);
        print_board(board);
        printf("Expected hash key: %llx\n", expected_hash);
        getchar();
    }
    */
    
    // Store position in repetition table to detect 3 fold repetition 
    board->repetition_index++;
    board->repetition_table[board->repetition_index] = board->hash_key;

    // Ensure King is not in Check
    if (is_square_attacked(get_least_sig_bit_index((board->side == WHITE) ? board->bitboards[k] : board->bitboards[K]), board->side, board)) {
        UNDO(board);
        return 0;
    }
    return 1;
}

int gives_check(Board *board, int move) {
    int src = MOVE_SRC(move);
    int target = MOVE_TARGET(move);
    int piece = MOVE_PIECE(move);
    int opponent = board->side;
    int king_square = get_least_sig_bit_index(board->bitboards[(opponent == WHITE) ? k : K]);

    // Check if the piece on the target square attacks the king
    if (piece == P || piece == p) {
        if (board->pawn_attacks[opponent][target] & (1ULL << king_square)) {
            return 1;
        }
    } else if (piece == N || piece == n) {
        if (board->knight_attacks[target] & (1ULL << king_square)) {
            return 1;
        }
    } else if (piece == B || piece == b) {
        if (get_bishop_attacks(target, board->occupancies[BOTH], board) & (1ULL << king_square)) {
            return 1;
        }
    } else if (piece == R || piece == r) {
        if (get_rook_attacks(target, board->occupancies[BOTH], board) & (1ULL << king_square)) {
            return 1;
        }
    } else if (piece == Q || piece == q) {
        if (get_queen_attacks(target, board->occupancies[BOTH], board) & (1ULL << king_square)) {
            return 1;
        }
    } else if (piece == K || piece == k) {
        if (board->king_attacks[target] & (1ULL << king_square)) {
            return 1;
        }
    }

    // Check for discovered checks
    if (piece == P || piece == p) {
        if (MOVE_ENPASSANT(move)) {
            int ep_capture_square = target + ((opponent == WHITE) ? 8 : -8);
            // Queen attacks are derived from rook and bishop attacks, so no need to check them.
            if (get_rook_attacks(src, board->occupancies[BOTH] ^ (1ULL << ep_capture_square), board) & (1ULL << king_square)) {
                return 1;
            }
            if (get_bishop_attacks(src, board->occupancies[BOTH] ^ (1ULL << ep_capture_square), board) & (1ULL << king_square)) {
                return 1;
            }
        }
    }

    // Check if moving the piece opens an attack from a sliding piece
    Bitboard occupancy_without_src = board->occupancies[BOTH] ^ (1ULL << src);
    if (get_rook_attacks(king_square, occupancy_without_src, board) & board->bitboards[(opponent == WHITE) ? R : r]) {
        return 1;
    }
    if (get_bishop_attacks(king_square, occupancy_without_src, board) & board->bitboards[(opponent == WHITE) ? B : b]) {
        return 1;
    }
    if (get_queen_attacks(king_square, occupancy_without_src, board) & board->bitboards[(opponent == WHITE) ? Q : q]) {
        return 1;
    }

    return 0;
}


void print_move(int move) {
    int promoted = MOVE_PROMOTED(move);
    if (promoted) {
        printf("%s%s%c", square[MOVE_SRC(move)], square[MOVE_TARGET(move)], promoted_pieces[promoted]);
        return;
    }
    printf("%s%s", square[MOVE_SRC(move)], square[MOVE_TARGET(move)]);
}

void print_move_list(Moves *move_list) {
    if (!move_list->count) {
        printf("\nNo moves\n");
        return;
    }
    printf("\n\tPiece\tMove   Capture\tDouble\tEP\tCastling\n");
    for (int i = 0; i < move_list->count; i++) {
        int move = move_list->moves[i];
    printf("%d.\t%c\t%s%s%c\t%d\t%d\t%d\t%d\n", 
                    i+1, 
                    ascii_pieces[MOVE_PIECE(move)], 
                    square[MOVE_SRC(move)], 
                    square[MOVE_TARGET(move)], 
                    MOVE_PROMOTED(move) ? promoted_pieces[MOVE_PROMOTED(move)] : ' ', 
                    MOVE_CAPTURE(move), 
                    MOVE_DOUBLE(move), 
                    MOVE_ENPASSANT(move),
                    MOVE_CASTLE(move));
    }
}