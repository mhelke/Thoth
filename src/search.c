#include <stdlib.h>
#include <stdio.h>

#include "search.h"
#include "eval.h"

int search(int depth) {
    Search search = {0};
    search.nodes = 0;
    int score = negamax(-INT_MAX, INT_MAX, depth, &search);
    printf("info score cp %d depth %d nodes %ld pv ", score, depth, search.nodes);
    for (int i = 0; i < search.pv_length[0]; i++) {
        print_move(search.pv_table[0][i]);
        printf(" ");
    }
    printf("\n");
    printf("bestmove ");
    print_move(search.pv_table[0][0]);
    printf("\n");
    return score;
}

int negamax(int alpha, int beta, int depth, Search *search) {
    search->pv_length[search->ply] = search->ply;
    
    if (depth == 0) {
        return quiescence(alpha, beta, search);
    }

    search->nodes++;

    int legal_move_count = 0;
    int check = is_square_attacked(get_least_sig_bit_index((side == WHITE) ? bitboards[K] : bitboards[k]), side ^ 1);

    if (check) {
        depth++;
    }

    Moves move_list[1];
    generate_moves(move_list);
    sort_moves(move_list, search);

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
            // Killer Heuristic
            if (MOVE_CAPTURE(move_list->moves[i]) == 0) {
                search->killer_moves[1][search->ply] = search->killer_moves[0][search->ply];
                search->killer_moves[0][search->ply] = move_list->moves[i];
            }
            return beta; // fails high
        }

        if (score > alpha) {
            // History Heuristic
            if (MOVE_CAPTURE(move_list->moves[i]) == 0) {
                search->history[MOVE_PIECE(move_list->moves[i])][MOVE_TARGET(move_list->moves[i])] += depth;
            }
            alpha = score; // PV node

            // Update PV line
            search->pv_table[search->ply][search->ply] = move_list->moves[i];
            for (int ply = search->ply + 1; ply < search->pv_length[search->ply + 1]; ply++) {
                search->pv_table[search->ply][ply] = search->pv_table[search->ply + 1][ply];
            }
            search->pv_length[search->ply] = search->pv_length[search->ply + 1];
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
    sort_moves(move_list, search);

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

int score_move(int move, Search *search) {

    if (!MOVE_CAPTURE(move)) {
        // Killer Heuristic
        if (search->killer_moves[0][search->ply] == move) {
            return BONUS_KILLER;
        }

        if (search->killer_moves[1][search->ply] == move) {
            return BONUS_SECOND_KILLER;
        }

        // History Heuristic
        return search->history[MOVE_PIECE(move)][MOVE_TARGET(move)];
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
    // 10,000 is added to ensure capture moves will score higher priority than quiet killer moves
    // This is because captures have a higher change of producing a cutoff.
    return mvv_lva[MOVE_PIECE(move)][captured_piece] + BONUS_CAPTURE;
}

void print_move_scores(Moves* move_list) {
    printf("Move Scores:\n");
    for (int i =0; i < move_list->count; i++) {
        printf("Move: ");
        print_move(move_list->moves[i]);
        Search search = {0};
        printf("  Score: %d\n", score_move(move_list->moves[i], &search));
    }
}

int sort_moves(Moves *move_list, Search *search) {
    int count = move_list->count;
    int *scores = (int *)malloc(count * sizeof(int));

    if (scores == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return -1;
    }

    for (int i = 0; i < count; i++) {
        scores[i] = score_move(move_list->moves[i], search);
    }

    for (int i = 0; i < count; i++) {
        for (int j = i + 1; j < count; j++) {
            if (scores[i] < scores[j]) {
                // Swap scores
                int temp_score = scores[i];
                scores[i] = scores[j];
                scores[j] = temp_score;

                // Swap moves
                int temp_move = move_list->moves[i];
                move_list->moves[i] = move_list->moves[j];
                move_list->moves[j] = temp_move;
            }
        }
    }
    free(scores);
    return 0;
}
