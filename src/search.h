#ifndef SEARCH_H
#define SEARCH_H

#include "move.h"

#define BONUS_KILLER 9000
#define BONUS_SECOND_KILLER 8000
#define BONUS_CAPTURE 10000

// int killer_moves[2][64]; 

// History
// int history[12][64]; // 
typedef struct {
    int ply;
    int best_move;
    unsigned long long nodes;
    int killer_moves[2][64]; // TODO: Increase plys for higher depths 
    int history[12][64];


} Search;

int search(int);
int negamax(int, int, int, Search*);
int quiescence(int, int, Search*);
void print_move_scores(Moves*);
int sort_moves(Moves*, Search*);

#endif
