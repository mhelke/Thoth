#include <stdlib.h>
#include <stdio.h>

#include "search.h"
#include "eval.h"
#include "move.h"

int search(int depth) {
    Search search[1] = {0};
    int score = negamax(-INT_MAX, INT_MAX, depth, search);
    printf("info score cp %d depth %d nodes %ld\n", score, depth, search->nodes);
    printf("bestmove ");
    print_move(search->best_move);
    printf("\n");
    return score;
}

int negamax(int alpha, int beta, int depth, Search *search) {
    if (depth == 0) {
        return quiescence(alpha, beta, search);
    }

    search->nodes++;
    int legal_move_count = 0;
    int check = is_square_attacked(get_least_sig_bit_index((side == WHITE) ? bitboards[K] : bitboards[k]), side ^ 1);

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

        legal_move_count++;

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

    // No moves - game over
    if (legal_move_count == 0) {
        // Checkmate
        if (check) {
            return -50000 + search->ply; // add ply to ensure the shortest possible checkmate is always found
        }

        // Stalemate Draw
        return 0;
    }

    if (prev_alpha != alpha) {
        search->best_move = current_best;
    }

    return alpha; // fails low
}

int quiescence(int alpha, int beta, Search *search) {

    search->nodes++;

    int score = evaluate();

     // Fail-hard
    if (score >= beta) {
        return beta; // fails high
    }

    if (score > alpha) {
        alpha = score; // PV 
    }


    Moves move_list[1];
    generate_moves(move_list);

    for (int i = 0; i < move_list->count; i++) {
        COPY_BOARD();
        search->ply++;

        // Only search captures to prevent the horizon effect.
        // Static evaluation should only be returned once the position has reached a quiet position.
        if (make_move(move_list->moves[i], CAPTURES) == 0) {
            // Not a capture
            search->ply--;
            UNDO();
            continue;
        }

        int score = -quiescence(-beta, -alpha, search);
        search->ply--;
        UNDO();

        // Fail-hard
        if (score >= beta) {
            return beta; // fails high
        }

        if (score > alpha) {
            alpha = score; // PV 
        }
    }

    return alpha; // fail low
}