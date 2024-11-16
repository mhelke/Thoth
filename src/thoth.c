/*
    Thoth Engine
    © Matthew Helke 2024
*/

#include <stdio.h>

#include "bitboard.h"

int main()
{
    U64 bitboard = 0ULL;

    set_bit(bitboard, e1);    

    print_bitboard(bitboard);

    pop_bit(bitboard, e1);
    print_bitboard(bitboard);

    return 0;
}