#include <stdio.h>

#include "table.h"
#include "magics.h"

Bitboard piece_keys[12][64];
Bitboard enpassant_keys[64];
Bitboard castling_keys[16];
Bitboard side_key;

void init_hash_keys() {
    for (int piece = P; piece <= k; piece++) {
        for (int square = 0; square < 64; square++) {
            piece_keys[piece][square] = generate_random_U64_number();
        }
    }
    for (int square = 0; square < 64; square++) {
        enpassant_keys[square] = generate_random_U64_number();
    }
    for (int i = 0; i < 16; i++) {
        castling_keys[i] = generate_random_U64_number();
    }   
    side_key = generate_random_U64_number();
}

Bitboard generate_hash_key(Board* board) {
    Bitboard key = 0ULL;

    Bitboard bitboard;
    
    for (int piece = P; piece <= k; piece++) {
        bitboard = board->bitboards[piece];

        while (bitboard) {
            int square = get_least_sig_bit_index(bitboard);
            
            // Hash board
            key ^= piece_keys[piece][square];    
            
            POP_BIT(bitboard, square);
        }
    }

    if (board->enpassant != na) {
        key ^= enpassant_keys[board->enpassant];
    }

    key ^= castling_keys[board->castle];
    if (board->side == BLACK) {
        key ^= side_key;
    }
    return key;
}