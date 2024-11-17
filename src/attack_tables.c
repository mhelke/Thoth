#include "bitboard.h"

// U64 board representation of all squares not on the A file
const U64 NOT_A_FILE = 18374403900871474942ULL;

// U64 board representation of all squares not on the H file
const U64 NOT_H_FILE = 9187201950435737471ULL;

const U64 NOT_HG_FILE = 4557430888798830399ULL;

const U64 NOT_AB_FILE = 18229723555195321596ULL;

// Pawn attacks table
U64 pawn_attacks[2][64];

// Knight attacks table
U64 knight_attacks[64];

// King attacks table
U64 king_attacks[64];


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

// Knight attacks

U64 mask_knight_attacks(int square) {
    // Attack bitboard
    U64 attacks = 0ULL;

    // Piece bitboard
    U64 bitboard = 0ULL;

    // Set piece on board
    set_bit(bitboard, square);

    // Generate kngiht moves.
    // Offsets 6, 10, 15, 17
    // Knights on A or B file should not jump to the H or G file 
    if ((bitboard >> 17) & NOT_H_FILE) {
        attacks |= (bitboard >> 17);
    }
    if ((bitboard >> 10) & NOT_HG_FILE) {
        attacks |= (bitboard >> 10);
    }
    // Knights on G or H file should not jump to the A or B
    if ((bitboard >> 15) & NOT_A_FILE) {
        attacks |= (bitboard >> 15);
    }
    if ((bitboard >> 6) & NOT_AB_FILE) {
        attacks |= (bitboard >> 6);
    }

    // Other direction
    if ((bitboard << 17) & NOT_A_FILE) {
        attacks |= (bitboard << 17);
    }
    if ((bitboard << 10) & NOT_AB_FILE) {
        attacks |= (bitboard << 10);
    }
    if ((bitboard << 15) & NOT_H_FILE) {
        attacks |= (bitboard << 15);
    }
    if ((bitboard << 6) & NOT_HG_FILE) {
        attacks |= (bitboard << 6);
    }

    return attacks;
}

U64 mask_king_attacks(int square) {
    // Attack bitboard
    U64 attacks = 0ULL;

    // Piece bitboard
    U64 bitboard = 0ULL;

    // Set piece on board
    set_bit(bitboard, square);

    // Generate king moves.
    if (bitboard >> 8) {
        attacks |= (bitboard >> 8);   
    }
    if (bitboard >> 9 & NOT_H_FILE) {
        attacks |= (bitboard >> 9);   
    }
    if (bitboard >> 7 & NOT_A_FILE) {
        attacks |= (bitboard >> 7);   
    }
    if (bitboard >> 1 & NOT_H_FILE) {
        attacks |= (bitboard >> 1);   
    }
    if (bitboard << 8) {
        attacks |= (bitboard << 8);   
    }
    if (bitboard << 9 & NOT_A_FILE) {
        attacks |= (bitboard << 9);   
    }
    if (bitboard << 7 & NOT_H_FILE) {
        attacks |= (bitboard << 7);   
    }
    if (bitboard << 1 & NOT_A_FILE) {
        attacks |= (bitboard << 1);   
    }

    return attacks;
}

U64 mask_bishop_attacks(int square) {
    // Attack bitboard
    U64 attacks = 0ULL;

    int rank, file;
    int target_rank = square / 8;
    int target_file = square % 8;

    // relevent bishop occupancy bits
    for (rank = target_rank + 1, file = target_file + 1; rank <= 6 && file <= 6; rank++, file++) {
        attacks |= (1ULL << (rank * 8 + file));
    }

    for (rank = target_rank - 1, file = target_file + 1; rank >= 1 && file <= 6; rank--, file++) {
        attacks |= (1ULL << (rank * 8 + file));
    }

    for (rank = target_rank + 1, file = target_file - 1; rank <= 6 && file >= 1; rank++, file--) {
        attacks |= (1ULL << (rank * 8 + file));
    }

     for (rank = target_rank - 1, file = target_file - 1; rank >= 1 && file >= 1; rank--, file--) {
        attacks |= (1ULL << (rank * 8 + file));
    }

    return attacks;


    // Set piece on board
    // set_bit(bitboard, square);
}

// Generate attack tables
void init_tables() {
    for (int square = 0; square < 64; square++) {
        // Pawn attacks
        pawn_attacks[white][square] = mask_pawn_attacks(white, square);
        pawn_attacks[black][square] = mask_pawn_attacks(black, square);

        // Kinght attacks
        knight_attacks[square] = mask_knight_attacks(square);

        // King attacks
        king_attacks[square] = mask_king_attacks(square);
    }
}