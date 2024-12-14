#include "bitboard.h"

extern Bitboard piece_keys[12][64];
extern Bitboard enpassant_keys[64];
extern Bitboard castling_keys[16];
extern Bitboard side_key;

void init_hash_keys();
Bitboard generate_hash_key(Board*);