#ifndef BITBOARD_H
#define BITBOARD_H

#include <string.h>
#include <stdint.h>

typedef unsigned long long Bitboard;

#define SET_BIT(bitboard, square) ((bitboard |= (1ULL << square)))
#define GET_BIT(bitboard, square) ((bitboard & (1ULL <<  square)))
#define POP_BIT(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define count_bits(bitboard) __builtin_popcountll(bitboard)
#define SQUARE_INDEX(rank, file) ((rank) * 8 + (file))

typedef struct {
    Bitboard bitboards[12];
    Bitboard occupancies[3];
    Bitboard pawn_attacks[2][64];
    Bitboard knight_attacks[64];
    Bitboard king_attacks[64];
    Bitboard *bishop_attacks;
    Bitboard *rook_attacks;
    Bitboard bishop_masks[64];
    Bitboard rook_masks[64];
    Bitboard hash_key;
    Bitboard repetition_table[1000];
    int side;
    int enpassant;
    int castle;
    int repetition_index; // Keep track of 3-fold repetition
    int fifty_move_rule_counter; // Keep track of the 50 move rule for draw (100 plys)
} Board;

#define COPY_BOARD(board)                                                                       \
    Bitboard bitboards_copy[12], occupancies_copy[3];                                           \
    int side_copy, enpassant_copy, castle_copy, fifty_move_rule_counter_copy;                   \
    memcpy(bitboards_copy, board->bitboards, sizeof(board->bitboards));                         \
    memcpy(occupancies_copy, board->occupancies, sizeof(board->occupancies));                   \
    side_copy = board->side, enpassant_copy = board->enpassant, castle_copy = board->castle;    \
    fifty_move_rule_counter_copy = board->fifty_move_rule_counter;                              \
    Bitboard hash_key_copy = board->hash_key;                                                   \

#define UNDO(board)                                                                             \
    memcpy(board->bitboards, bitboards_copy, 96);                                               \
    memcpy(board->occupancies, occupancies_copy, 24);                                           \
    board->side = side_copy, board->enpassant = enpassant_copy, board->castle = castle_copy;    \
    board->repetition_index--;                                                                  \
    board->fifty_move_rule_counter = fifty_move_rule_counter_copy;                              \
    board->hash_key = hash_key_copy;                                                            \

Bitboard mask_pawn_attacks(int, int);
Bitboard mask_knight_attacks(int);
Bitboard mask_king_attacks(int);
Bitboard mask_bishop_attacks(int);
Bitboard mask_rook_attacks(int);
Bitboard generate_bishop_attacks(int, Bitboard);
Bitboard generate_rook_attacks(int, Bitboard);
Bitboard set_occupancy(int, int, Bitboard);
Bitboard get_bishop_attacks(int, Bitboard, Board*); 
Bitboard get_rook_attacks(int, Bitboard, Board*); 
Bitboard get_queen_attacks(int, Bitboard, Board*); 

Board* create_board();
void free_board(Board *board);
void init_siders(int, Board*);
void init_tables(Board*);
void print_bitboard(Bitboard);

// get least significant 1st bit index
static inline int get_least_sig_bit_index(Bitboard bitboard) {
    if (!bitboard) {
        return -1; // illegal index
    }
    return __builtin_ctzll(bitboard);
}

static inline int get_piece_at_square(int square, Bitboard bitboards[]) {
    for (int piece = 0; piece < 12; piece++) {
        if (GET_BIT(bitboards[piece], square)) {
            return piece;
        }
    }
    return -1;
}

// Array representing each index as the human-readable chess board coordinate
// Required to follow UCI guidelines for moves
static const char *square[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

// Board representation
enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, 
    na,
};

// Castling
enum { WK = 1, WQ = 2, BK = 4, BQ = 8 };

// Pieces
enum { P, N, B, R, Q, K, p, n, b, r, q, k };

// Side to move
enum { WHITE, BLACK, BOTH };
enum { sROOK, sBISHOP };

extern const Bitboard bishop_magics[64];
extern const Bitboard rook_magics[64];
extern const int bishop_relevant_bits[64];
extern const int rook_relevant_bits[64];
extern char ascii_pieces[12];
extern int char_pieces[];

#endif