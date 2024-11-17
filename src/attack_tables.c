#include "bitboard.h"

// U64 board representation of all squares not on the A file
const U64 NOT_A_FILE = 18374403900871474942ULL;

// U64 board representation of all squares not on the H file
const U64 NOT_H_FILE = 9187201950435737471ULL;

const U64 NOT_HG_FILE = 4557430888798830399ULL;

const U64 NOT_AB_FILE = 18229723555195321596ULL;

// Pawn attacks table
U64 pawn_attacks[2][64];


// Pawn attacks
U64 mask_pawn_attacks(int side, int square) {
    // Attack bitboard
    U64 attacks = 0ULL;

    // Piece bitboard
    U64 bitboard = 0ULL;

    // Set piece on board
    set_bit(bitboard, square);

    // white = 0, black = 1
    // pawn captures offset of 7 & 9
    if (!side) {
        // Pawns on a-file can only capture on the b-file 
        if((bitboard >> 7) & NOT_A_FILE) {
            attacks |= (bitboard >> 7);
        }
        // Pawns on h-file can only capture on the g-file
        if((bitboard >> 9) & NOT_H_FILE) {
            attacks |= (bitboard >> 9);
        }
    } else {
        // Pawns on a-file can only capture on the b-file 
        if((bitboard << 7) & NOT_H_FILE) {
            attacks |= (bitboard << 7);
        }
        // Pawns on h-file can only capture on the g-file
        if((bitboard << 9) & NOT_A_FILE) {
            attacks |= (bitboard << 9);
        }
    }

    // return attack table
    return attacks;

}

// Generate pawn attacks
void init_pawn_attacks() {
    for (int square = 0; square < 64; square++) {
        // Pawn attacks
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);
    }
}

void init_tables() {
    init_pawn_attacks();
}