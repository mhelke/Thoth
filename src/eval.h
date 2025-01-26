#ifndef EVAL_H
#define EVAL_H

#include "bitboard.h"

int evaluate(Board*);
void init_evaluation_masks();
void printEval(Board*);

extern Bitboard file_masks[64];
extern Bitboard rank_masks[64];
extern Bitboard isolated_masks[64];
extern Bitboard white_passed_masks[64];
extern Bitboard black_passed_masks[64];

#endif