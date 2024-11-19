#include "bitboard.h"

Bitboard mask_pawn_attacks(int, int);
Bitboard mask_knight_attacks(int);
Bitboard mask_king_attacks(int);
Bitboard mask_bishop_attacks(int);
Bitboard mask_rook_attacks(int);
Bitboard generate_bishop_attacks(int, Bitboard);
Bitboard generate_rook_attacks(int, Bitboard);
Bitboard set_occupancy(int, int, Bitboard);

extern const int bishop_relevant_bits[64];
extern const int rook_relevant_bits[64];

void init_tables();