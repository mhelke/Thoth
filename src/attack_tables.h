#include "bitboard.h"

U64 mask_pawn_attacks(int, int);
U64 mask_knight_attacks(int);
U64 mask_king_attacks(int);
U64 mask_bishop_attacks(int);
U64 mask_rook_attacks(int);
void init_tables();