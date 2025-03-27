#include <stdio.h>

#include "table.h"
#include "magics.h"

Bitboard piece_keys[12][64];
Bitboard enpassant_keys[64];
Bitboard castling_keys[16];
Bitboard side_key;

int hash_entries = 0;
TranspositionTable *transposition_table = NULL; 

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

void init_hash_table(int mb) {
    if (mb <= 0) {
        printf("    [ERROR] Invalid memory size for hash table allocation!\n");
        mb = 64;
    }

    int hash_size = 0x100000 * mb;
    hash_entries = hash_size / sizeof(TranspositionTable);

    if (transposition_table != NULL) {
        free(transposition_table);
    }

    transposition_table = (TranspositionTable *) malloc(hash_entries * sizeof(TranspositionTable));
    
    if (transposition_table == NULL) {
        printf("    [ERROR] Error allocating hash table with %dMB!\n", mb);
        init_hash_table(mb / 2);
        return;
    }
    clear_transposition_table();
    printf("    [DEBUG] Hash table allocated with %dMB and %d entries\n", mb, hash_entries);
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
    printf("    [DEBUG] Clearing transposition table\n");
    memset(transposition_table, 0, hash_entries * sizeof(TranspositionTable));
}

void record_hash(Board* board, int score, int depth, int ply, int flag) {
    if (score < -MATE_SCORE) score -= ply;
    if (score > MATE_SCORE) score += ply;
    TranspositionTable *entry = &transposition_table[board->hash_key % hash_entries];
    entry->key = board->hash_key;
    entry->score = score;
    entry->flag = flag;
    entry->depth = depth; 
}

int probe_hash(Board* board, int alpha, int beta, int depth, int ply) {
    TranspositionTable *entry = &transposition_table[board->hash_key % hash_entries];

    if (entry->key != board->hash_key) {
        return valueUNKNOWN;
    }

    if (entry->depth < depth) {
        return valueUNKNOWN;
    }

    int score = entry->score;
    if (score < -MATE_SCORE) score += ply;
    if (score > MATE_SCORE) score -= ply;

    if (entry->flag == flagEXACT) {
        return score;
    }
    if ((entry->flag == flagALPHA) && (score <=alpha)) {
        return alpha;
    }
    if ((entry->flag == flagBETA) && (score >= beta)) {
        return beta;
    }

    return valueUNKNOWN;

}