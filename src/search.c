#include <stdlib.h>
#include <stdio.h>

#include "search.h"
#include "eval.h"
#include "move.h"

int search(int depth) {
    Search search[1] = {0};
    int score = negamax(-INT_MAX, INT_MAX, depth, search);
    printf("bestmove ");
    print_move(search->best_move);
    printf("\n");
    return score;
}

int negamax(int alpha, int beta, int depth, Search *search) {
    if (depth == 0) {
        return evaluate();
    }

    search->nodes++;
    int current_best;
    int prev_alpha = alpha;

    Moves move_list[1];
    generate_moves(move_list);

    for (int i = 0; i < move_list->count; i++) {
        COPY_BOARD();
        search->ply++;

        if (make_move(move_list->moves[i], ALL_MOVES) == 0) {
            // illegal move
            search->ply--;
            UNDO();
            continue;
        }
        int score = -negamax(-beta, -alpha, depth-1, search);
        search->ply--;
        UNDO();

        // Fail-hard
        if (score >= beta) {
            return beta; // fails high
        }

        if (score > alpha) {
            alpha = score; // PV 

            if (search->ply == 0) {
                current_best = move_list->moves[i];
            }
        }
    }

    if (prev_alpha != alpha) {
        search->best_move = current_best;
    }

    return alpha; // fails low
}