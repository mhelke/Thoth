#include "bitboard.h"

U64 mask_pawn_attacks(int, int);
U64 mask_knight_attacks(int);
U64 mask_king_attacks(int);
U64 mask_bishop_attacks(int);
U64 mask_rook_attacks(int);
U64 generate_bishop_attacks(int, U64);
U64 generate_rook_attacks(int, U64);
U64 set_occupancy(int, int, U64);
// The number of relevant occupancy squares within the path of the bishop's attack mask.
// See [gen.c] for how this was calculated.
static const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6,
};

// The number of relevant occupancy squares within the path of the rook's attack mask.
// See [gen.c] for how this was calculated.
static const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12,
};

void init_tables();