#ifndef BITBOARD_H
#define BITBOARD_H

#include <string.h>

typedef unsigned long long Bitboard;

#define SET_BIT(bitboard, square) ((bitboard |= (1ULL << square)))
#define GET_BIT(bitboard, square) ((bitboard & (1ULL <<  square)))
#define POP_BIT(bitboard, square) ((bitboard) &= ~(1ULL << (square)))
#define SQUARE_INDEX(rank, file) ((rank) * 8 + (file))

typedef struct {
    Bitboard bitboards[12];
    Bitboard occupancies[3];
    Bitboard pawn_attacks[2][64];
    Bitboard knight_attacks[64];
    Bitboard king_attacks[64];
    Bitboard bishop_attacks[64][512];
    Bitboard rook_attacks[64][4096];
    Bitboard bishop_masks[64];
    Bitboard rook_masks[64];
    int side;
    int enpassant;
    int castle;
    Bitboard hash_key;
} Board;

#define COPY_BOARD(board)                                                    \
    Bitboard bitboards_copy[12], occupancies_copy[3];                   \
    int side_copy, enpassant_copy, castle_copy;                         \
    memcpy(bitboards_copy, board->bitboards, sizeof(board->bitboards));               \
    memcpy(occupancies_copy, board->occupancies, sizeof(board->occupancies));         \
    side_copy = board->side, enpassant_copy = board->enpassant, castle_copy = board->castle; \
    Bitboard hash_key_copy = board->hash_key;                                           \

#define UNDO(board)                                                          \
    memcpy(board->bitboards, bitboards_copy, 96);                              \
    memcpy(board->occupancies, occupancies_copy, 24);                          \
    board->side = side_copy, board->enpassant = enpassant_copy, board->castle = castle_copy; \
    board->hash_key = hash_key_copy                                                           \

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

// count total bits in the bitboard  
static inline int count_bits(Bitboard bitboard) {
    // bit counter
    int count = 0;

    while (bitboard > 0) {
        count++;
        // reset least significant 1st bit
        bitboard &= bitboard - 1;
    }
    return count;
}

// get least significant 1st bit index
static inline int get_least_sig_bit_index(Bitboard bitboard) {
    // (block_board & -block_board) - 1
    if (!bitboard) {
        return -1; // illegal index
    }
    return count_bits((bitboard & -bitboard) - 1);
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

extern char ascii_pieces[12];
extern int char_pieces[];

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
enum { ROOK, BISHOP };

extern const Bitboard bishop_magics[64];
extern const Bitboard rook_magics[64];
extern const int bishop_relevant_bits[64];
extern const int rook_relevant_bits[64];

#endif