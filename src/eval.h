#ifndef EVAL_H
#define EVAL_H

#include "bitboard.h"

int evaluate(Board*);
void init_evaluation_masks();
void printEval(Board*);
int get_material(int);

extern Bitboard file_masks[64];
extern Bitboard rank_masks[64];
extern Bitboard isolated_masks[64];
extern Bitboard white_passed_masks[64];
extern Bitboard black_passed_masks[64];

// Material scores indexed by the piece type [PAWN, KNIGHT, BISHIOP, ROOK, QUEEN, KING]
static const int MATERIAL_SCORE[6] = { 100, 325, 335, 500, 975, 0 };

// The threshold of material where the endgame phase begins
static const int ENDGAME_MATERIAL_THRESHOLD = 1300;

#endif