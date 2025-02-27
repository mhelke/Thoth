#ifndef SEARCH_H
#define SEARCH_H

#include "move.h"
#include "bitboard.h"

#define BONUS_KILLER 9000
#define BONUS_SECOND_KILLER 8000
#define BONUS_CAPTURE 10000
#define MAX_PLY 64
#define FULL_DEPTH_MOVES 4
#define REDUCTION_LIMIT 3
#define NULL_REDUCTION_LIMIT 4
#define REDUCTION 2
#define NULL_REDUCTION 3
#define ASPIRATION_WINDOW 50
#define DRAW_SCORE 0
#define DELTA_PRUNE_MARGIN 200
#define FUTILITY_MARGIN 200
#define RAZOR_MARGIN 100
#define TEMPO_BONUS 10

typedef struct {
    int ply;
    // int best_move;
    unsigned long long nodes;
    int killer_moves[2][MAX_PLY]; // TODO: Increase plys for higher depths 
    int history[12][64]; // [piece][square]
    int pv_length[MAX_PLY]; // TODO: Increase plys for higher depths
    int pv_table[MAX_PLY][MAX_PLY]; // TODO: Increase plys for higher depth
    int follow_pv;
    int score_pv;
    int stopped;
    Board *board;
} Search;

int search(int, Board*);
int negamax(int, int, int, Search*);
int quiescence(int, int, Search*);
void print_move_scores(Moves*);
int sort_moves(Moves*, Search*);
void pv_scoring(Moves*, Search*);
int is_repetition(Board*);
int see(Board*, int, int);
int get_smallest_attacker(Bitboard, int, Bitboard[]);

#endif
