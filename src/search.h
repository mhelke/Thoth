#ifndef SEARCH_H
#define SEARCH_H

#include "move.h"

typedef struct {
    int ply;
    int best_move;
    unsigned long long nodes;
} Search;

int search(int);
int negamax(int, int, int, Search*);
int quiescence(int, int, Search*);
void print_move_scores(Moves*);
int sort_moves(Moves*);

#endif
