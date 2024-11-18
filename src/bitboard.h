#ifndef BITBOARD_H
#define BITBOARD_H

#define U64 unsigned long long

// set
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))

// get
#define get_bit(bitboard, square) (bitboard & (1ULL <<  square))

// pop
#define pop_bit(bitboard, square) (get_bit(bitboard, square) ? bitboard ^= (1ULL << square) : 0)

// count total bits in the bitboard  
static inline int count_bits(U64 bitboard) {
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
static inline int get_least_sig_bit_index(U64 bitboard) {
    // (block_board & -block_board) - 1
    if (!bitboard) {
        return -1; // illegal index
    }
    return count_bits((bitboard & -bitboard) - 1);
}


void print_bitboard(U64 bitboard);

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
};

// Side to move
enum { white, black };
#endif