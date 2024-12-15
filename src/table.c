#include <stdio.h>

#include "table.h"
#include "magics.h"

Bitboard piece_keys[12][64];
Bitboard enpassant_keys[64];
Bitboard castling_keys[16];
Bitboard side_key;

TranspositionTable transposition_table[hashSIZE]; 

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

void clear_transposition_table() {
    for (int i = 0; i < hashSIZE; i++) {
        transposition_table[i].key = 0;
        transposition_table[i].depth = 0;
        transposition_table[i].flag = 0;
        transposition_table[i].score = 0;
        transposition_table[i].best_move = 0;
    }
}

void record_hash(Board* board, int score, int depth, int flag) {
    TranspositionTable *entry = &transposition_table[board->hash_key % hashSIZE];
    entry->key = board->hash_key;
    entry->score = score;
    entry->flag = flag;
    entry->depth = depth; 
}

int probe_hash(Board* board, int alpha, int beta, int depth) {
    TranspositionTable *entry = &transposition_table[board->hash_key % hashSIZE];

    if (entry->key == board->hash_key) {
        if (entry->depth >= depth) {
            if (entry->flag == flagEXACT) {
                return entry->score;
            }
            if ((entry->flag == flagALPHA) && (entry->score <=alpha)) {
                return alpha;
            }
            if ((entry->flag == flagBETA) && (entry->score >= beta)) {
                return beta;
            }
        }
    }

    return valueUNKNOWN;

}