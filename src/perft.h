#ifndef PERFT_H
#define PERFT_H

#include "bitboard.h"
#include "board.h"

void perft(int, Board*);
void perft_test(char*, int, const unsigned long long*, Board*);
void perft_tests();
#endif