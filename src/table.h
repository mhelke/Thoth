#include <stdlib.h>
#include "bitboard.h"

#define hashSIZE 80000 // TODO: This should dynamically allocate memory
#define valueUNKNOWN INT_MAX

#define flagEXACT 0
#define flagALPHA 1
#define flagBETA 2

#define MATE_VALUE 50000
#define MATE_SCORE 49000

typedef struct {
    Bitboard key;
    int depth;
    int flag;
    int score;
    int best_move;
} TranspositionTable;

extern TranspositionTable transposition_table[hashSIZE]; 

extern Bitboard piece_keys[12][64];
extern Bitboard enpassant_keys[64];
extern Bitboard castling_keys[16];
extern Bitboard side_key;

void init_hash_keys();
Bitboard generate_hash_key(Board*);
void clear_transposition_table();
void record_hash(Board*, int, int, int, int);
int probe_hash(Board*, int, int, int, int);