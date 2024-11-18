#include "bitboard.h"

U64 mask_pawn_attacks(int, int);
U64 mask_knight_attacks(int);
U64 mask_king_attacks(int);
U64 mask_bishop_attacks(int);
U64 mask_rook_attacks(int);
U64 generate_bishop_attacks(int, U64);
U64 generate_rook_attacks(int, U64);
U64 set_occupancy(int, int, U64);
void init_tables();