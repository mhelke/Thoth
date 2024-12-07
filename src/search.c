#include <stdlib.h>
#include <stdio.h>

#include "search.h"
#include "eval.h"

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

/******** Most valuable victim/least valuable attacker (MVV-LVA) ********
 * When a less valuable piece captures a more valuable piece, 
 * the move should have priority in the search order (eg PxQ).
 * This helps hit the beta cutoff sooner, reducing the nodes searched. 
 *                           
 *  (Victim)    Pawn Knight Bishop   Rook  Queen   King
 *  (Attacker)
 *        Pawn   105    205    305    405    505    605
 *      Knight   104    204    304    404    504    604
 *      Bishop   103    203    303    403    503    603
 *        Rook   102    202    302    402    502    602
 *       Queen   101    201    301    401    501    601
 *        King   100    200    300    400    500    600
 *   
 ***********************************************************************/

// MVV LVA [attacker][victim]
static int mvv_lva[12][12] = {
 	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};


int score_move(int move) {

    if (!MOVE_CAPTURE(move)) {
        return 0;
    }

    int captured_piece = P;

    int start = (side == WHITE) ? p : P;
    int end = (side == WHITE) ? k : K;
    int target = MOVE_TARGET(move);

    for (int piece = start; piece <= end; piece++) {
        if (GET_BIT(bitboards[piece], target)) {
            captured_piece = piece;
            break;
        }
    }
    // MVV-LVA lookup
    return mvv_lva[MOVE_PIECE(move)][captured_piece];
}

void print_move_scores(Moves* move_list) {
    printf("Move Scores:\n");
    for (int i =0; i < move_list->count; i++) {
        printf("Move: ");
        print_move(move_list->moves[i]);
        printf("  Score: %d\n", score_move(move_list->moves[i]));
    }
}