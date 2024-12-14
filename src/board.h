#ifndef BOARD_H
#define BOARD_H

#include "bitboard.h"

#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

void load_fen(char*, Board*);
void reset_board(Board*);
void print_board(Board*);

#endif