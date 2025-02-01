#include <stdlib.h>
#include <stdio.h>

#include "search.h"
#include "eval.h"
#include "util.h"
#include "uci.h"
#include "table.h"
#define MAX(a, b) ((a) > (b) ? (a) : (b))
int total_researches = 0, hash_hits = 0, beta_cutoff_count = 0;

int search(int depth, Board *board) {
    int start = get_ms();
    int score = 0;
    Search search = {0};
    search.nodes = 0;
    search.score_pv = 0;
    search.follow_pv = 0;
    search.stopped = 0;
    search.board = board;

    int alpha = -INT_MAX;
    int beta = INT_MAX;
    int current_depth = 1;
    // Iterative deepening
    while (current_depth <= depth) {

        if (search.stopped) {
            break;
        }

        search.follow_pv = 1;
        score = negamax(alpha, beta, current_depth, &search);

        // Aspiration Window
        // Note - watch for search instability with this and adjust as needed.
        // Assume the score in the next iteration is not likely to be much different from this iteration's score.
        // This produces more beta cutoffs, leading to reduced nodes searched.

        // Score falls outside the window. Search again at full depth.
        if ((score <= alpha) || (score >= beta)) {
            alpha = -INT_MAX;
            beta = INT_MAX;
            total_researches++;
            continue;
        }
        // Apply aspiration window
        alpha = score - ASPIRATION_WINDOW;
        beta = score + ASPIRATION_WINDOW;

        if (search.pv_length[0]) {
            if (score > -MATE_VALUE && score < -MATE_SCORE) {
                printf("info score mate %d depth %d nodes %lld time %d pv ", -(score + MATE_VALUE) / 2 - 1, current_depth, search.nodes, get_ms() - start);
            } else if (score > MATE_SCORE && score < MATE_VALUE) {
                printf("info score mate %d depth %d nodes %lld time %d pv ", (MATE_VALUE - score) / 2 + 1, current_depth, search.nodes, get_ms() - start);   
            } else {
                printf("info score cp %d depth %d nodes %lld time %d pv ", score, current_depth, search.nodes, get_ms() - start);
            }
            for (int i = 0; i < search.pv_length[0]; i++) {
                print_move(search.pv_table[0][i]);
                printf(" ");
            }
            printf("\n");
            current_depth++;
        }
    }
    printf("bestmove ");
    print_move(search.pv_table[0][0]);
    printf("\n");
    printf("    [DEBUG] Total Time: %d\n", (get_ms() - start));
    printf("    [DEBUG] Total Full Re-searches: %d\n", total_researches);
    printf("    [DEBUG] Hash hits: %d\n", hash_hits);
    printf("    [DEBUG] Beta Cut-offs: %d\n", beta_cutoff_count);

    return score;
}

int negamax(int alpha, int beta, int depth, Search *search) {
    Board *board = search->board;

    int score;

    // Set flag to alpha unless a node is found that outscores alpha.
    int hash_flag = flagALPHA;

    /** 
     * 3-fold repetition or 50-move rule draw 
     * 
     * Note: There is no need to handle adding a move to the PV table here because if the game is a draw on the first node,
     * the GUI will handle drawing the game itself. Essentially, the GUI will never ask to search a position
     * that is already a draw by the 50-move rule or has been repeated 3 times.
     * This condition is only needed to inform the engine that the current node is a draw. In this case, there will already be a PV node.
    */
    if ((search->ply && is_repetition(board)) || board->fifty_move_rule_counter >= 100) {
        return DRAW_SCORE;
    }

    // If the move was already searched, return the score from the previous search
    // Only read from the hash table if it is not the root node and not the pv node.
    // In PVS, the PV node is defined as beta - alpha > 1.
    int is_pv = beta-alpha > 1;
    if (search->ply && !is_pv && (score = probe_hash(board, alpha, beta, depth, search->ply)) != valueUNKNOWN) {
        hash_hits++;
        return score;
    }

    // Check for UCI input
    if ((search->nodes & 2047) == 0) {
        search->stopped = should_stop();
    }

    int moves_searched = 0;

    search->pv_length[search->ply] = search->ply;
    
    if (depth == 0) {
        return quiescence(alpha, beta, search);
    }

    if (search->ply >= MAX_PLY) {
        // Too deep in search
        return evaluate(board);
    }

    search->nodes++;

    int legal_move_count = 0;
    int check = is_square_attacked(get_least_sig_bit_index((board->side == WHITE) ? board->bitboards[K] : board->bitboards[k]), board->side ^ 1, board);

    if (check) {
        depth++;
    }

    // Null Move Pruning
    if (depth >= REDUCTION_LIMIT && !check && search->ply) {
        COPY_BOARD(board);
        search->ply++;
        board->repetition_index++;
        board->repetition_table[board->repetition_index] = board->hash_key;

        // Give opponent a "null" move
        if (board->enpassant != na) board->hash_key ^= enpassant_keys[board->enpassant];
        board->enpassant = na;
        board->side ^= 1;
        board->hash_key ^= side_key;
        
        // Search with reduced depth to find early beta-cutoffs.
        score = -negamax(-beta, -beta+1, depth-NULL_REDUCTION, search);

        search->ply--;
        UNDO(board);

        if (search->stopped) {
            return 0;
        }

        if (score >= beta) {
            // Node fails high beta-cutoff
            beta_cutoff_count++;
            return beta;
        }
    }

    Moves move_list[1];
    generate_moves(move_list, board);

    if (search->follow_pv) {
        pv_scoring(move_list, search);
    }

    sort_moves(move_list, search);

    for (int i = 0; i < move_list->count; i++) {
        COPY_BOARD(board);
        search->ply++;

        if (make_move(move_list->moves[i], board) == 0) {
            // illegal move
            search->ply--;
            continue;
        }
        legal_move_count++;
         
        int score;

        // First move, run full-depth alpha-beta search
        if (moves_searched == 0) {
            score = -negamax(-beta, -alpha, depth-1, search);
        } else {
            // Late Move Reduction (LMR)
            if (moves_searched >= FULL_DEPTH_MOVES && depth >= REDUCTION_LIMIT && can_reduce(check, move_list->moves[i])) {
                // Search with a reduced depth
                score = -negamax(-alpha-1, -alpha, depth-REDUCTION, search);
            } else {
                // Ensures search is performed below
                score = alpha + 1;
            }
            // Better move found
            if (score > alpha) {
                // Once a move is found that is between alpha and beta, the goal is to prove that the rest of the moves are all bad.
                // This is generally faster than trying to find a move withing the remaining moves that might be good.
                // Search the remaining moves with full-depth, smaller window
                score = -negamax(-alpha-1, -alpha, depth-1, search);
                // If this assumption was proven wrong, and there is a better move than the initial PV move,
                // search again using a standard alpha-beta search.
                // This scenario actually wastes time by needing to search again, but it generally doesn't
                // happen enough to cancel out the time saved when the assumption is correct.
                // LMR failed. Re-search with regular alpha-beta search
                if (score > alpha && score < beta) {
                    total_researches++;
                    // Assumption was wrong - re-search the move with standard alpha-beta search.
                    score = -negamax(-beta, -alpha, depth-1, search);
                }
            }
        }

        search->ply--;
        UNDO(board);

         if (search->stopped) {
            return 0;
        }

        moves_searched++;

        if (score > alpha) {
            hash_flag = flagEXACT; 
            // History Heuristic
            if (MOVE_CAPTURE(move_list->moves[i]) == 0) {
                search->history[MOVE_PIECE(move_list->moves[i])][MOVE_TARGET(move_list->moves[i])] += depth;
            }
            alpha = score; // Found PV node

            // Update PV line
            search->pv_table[search->ply][search->ply] = move_list->moves[i];
            for (int ply = search->ply + 1; ply < search->pv_length[search->ply + 1]; ply++) {
                search->pv_table[search->ply][ply] = search->pv_table[search->ply + 1][ply];
            }
            search->pv_length[search->ply] = search->pv_length[search->ply + 1];
        }

        // Fail-hard beta-cutoff
        if (score >= beta) {
            beta_cutoff_count++;
            record_hash(board, beta, depth, search->ply, flagBETA);
            // Killer Heuristic
            if (MOVE_CAPTURE(move_list->moves[i]) == 0) {
                search->killer_moves[1][search->ply] = search->killer_moves[0][search->ply];
                search->killer_moves[0][search->ply] = move_list->moves[i];
            }
            return beta; // fails high
        }
    }

    // No moves - game over
    if (legal_move_count == 0) {
        // Checkmate
        if (check) {
            return -MATE_VALUE + search->ply; // add ply to ensure the shortest possible checkmate is always found
        }

        // Stalemate Draw
        return DRAW_SCORE;
    }

    record_hash(board, alpha, depth, search->ply, hash_flag);
    return alpha; // fails low
}

/**
 * Quiescence Search
 * 
 * Quiescence search is a specialized search that ensures only quiet positions are evaluated.
 * This is done to prevent the horizon effect, where the engine stops searching at a position that is unstable.
 * The goal is to search through all sequences of captures until a quiet position is reached, and only then evaluate the position.
 * 
 * The hoizon effect must be handled to prevent inaccurate scores. For example:
 * If the search stops after a capture, say PxQ, the evaluation function may think one side is up a pawn, but in reality, 
 * if you were to search 1 move deeper, you would see the opponent can recaputre the queen with a pawn, leading to a loss of material.
 */
int quiescence(int alpha, int beta, Search *search) {
    Board *board = search->board;

    // Check for UCI input
    if ((search->nodes & 2047) == 0) {
        search->stopped = should_stop();
    }

    search->nodes++;

    // Too deep in the search
    if (search->ply >= MAX_PLY) {
        return evaluate(board);
    }

    int score = evaluate(board);
    int stand_pat = score;

    if (score > alpha) {
        alpha = score; // PV node 
    }

     // Fail-hard
    if (score >= beta) {
        return beta; // fails high cut-node
    }

    Moves move_list[1];
    generate_moves(move_list, board);
    sort_moves(move_list, search); // remove?

    int opponent_material = get_material(!board->side);
    for (int i = 0; i < move_list->count; i++) {
        // Only search captures       
        if (!MOVE_CAPTURE(move_list->moves[i])) continue;

        /*
            Delta Cutoff

            If the move gaurentees a score far below alpha, it is not worth searching.
            This is determined by taking the evaluation + the value of the captured piece + a 200 centi-pawn margin. 
            If that value is still not enough to raise alpha, the position is likely lost.
            
            This technique should not be used in the endgame.
        */

        // Do not prune a capture on promotion. The position may be unstable.
        if (!MOVE_PROMOTED(move_list->moves[i])) {
            int captured_piece = -1;

            if (MOVE_ENPASSANT(move_list->moves[i])) {
                captured_piece = (board->side == WHITE) ? p : P;
            } else {
                int start = (board->side == WHITE) ? p : P;
                int end = (board->side == WHITE) ? k : K;
                int target = MOVE_TARGET(move_list->moves[i]);

                for (int piece = start; piece <= end; piece++) {
                    if (GET_BIT(board->bitboards[piece], target)) {
                        captured_piece = piece;
                        break;
                    }
                }
            }

            if (captured_piece == -1) {
                printf("    [ERROR]: Captured piece not found\n");
                continue;
            }

            int captured_piece_value = MATERIAL_SCORE[captured_piece % 6];

            // Delta Cutoff - if the capture is guarenteed to not raise alpha, and it is not the endgame, prune the move.
            if ((stand_pat + captured_piece_value + DELTA_PRUNE_MARGIN < alpha) && (opponent_material - captured_piece_value > ENDGAME_MATERIAL_THRESHOLD)) {
                continue;
            }

            // Static Exchange Evaluation (SEE) - if a capture sequence loses material, prune the move.
            if (see(board, MOVE_TARGET(move_list->moves[i]), MOVE_PIECE(move_list->moves[i]), MOVE_SRC(move_list->moves[i])) < 0) {
                continue;
            }
        }

        COPY_BOARD(board);
        search->ply++;

        if (make_move(move_list->moves[i], board) == 0) {
            search->ply--;
            continue;
        }

        int score = -quiescence(-beta, -alpha, search);
        search->ply--;
        UNDO(board);

        if (search->stopped) {
            return 0;
        }

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

// Static Exchange Evaluation (SEE)
int see(Board *board, int target_square, int pieces, int from_sq) {
    int side = board->side;
    
    // All pieces that can attack the target square
    Bitboard attackers = get_attackers_to_square(target_square, side, board->occupancies, board->bitboards, board);
    Bitboard defenders = get_attackers_to_square(target_square, side^1, board->occupancies, board->bitboards, board);
    Bitboard attacks_and_defneds = attackers | defenders;
    
    // Gain array
    int gains[32], d = 0;

    // Initial gain from capture
    gains[d] = MATERIAL_SCORE[get_piece_at_square(target_square, board->bitboards) % 6];

    int src = from_sq;
    int piece = get_piece_at_square(src, board->bitboards);

    // Create copies of occupancy and bitboards
    Bitboard occupancies[3];
    Bitboard bitboards[12];
    for (int i = 0; i < 3; i++) occupancies[i] = board->occupancies[i];
    for (int i = P; i <= k; i++) bitboards[i] = board->bitboards[i];

    do {

        // // If the king can take, but there are other attackers, the king is not considered an attacker.
        if (piece == (side == WHITE ? K : k) && get_attackers_to_square(target_square, !side, occupancies, bitboards, board) ) {
            POP_BIT(attacks_and_defneds, src);
            POP_BIT(bitboards[piece], src);
            POP_BIT(occupancies[BOTH], src);
            POP_BIT(occupancies[side^1], src);
            POP_BIT(occupancies[side], src);
            src = get_smallest_attacker(attacks_and_defneds, side, bitboards);
            piece = get_piece_at_square(src, bitboards); 
        }

        // Update pieces that can attack the target square to account for possible x-ray attacks
        attackers = get_attackers_to_square(target_square, side, occupancies, bitboards, board);
        defenders = get_attackers_to_square(target_square, side^1, occupancies, bitboards, board);
        attacks_and_defneds = attackers | defenders;

        d++;
        side = side^1;

        gains[d] = MATERIAL_SCORE[piece % 6] - gains[d-1];

        POP_BIT(attacks_and_defneds, src);
        POP_BIT(bitboards[piece], src);
        POP_BIT(occupancies[BOTH], src);
        POP_BIT(occupancies[side^1], src);
        POP_BIT(occupancies[side], src);

        // Find the least valuable attacker to capture next
        src = get_smallest_attacker(attacks_and_defneds, side, bitboards);
        piece = get_piece_at_square(src, bitboards); 
    } while (attacks_and_defneds);

    // Gain propagation
    while (--d) {
        gains[d-1] = -MAX(-gains[d-1], gains[d]);
    }
    return gains[0];
}

// Get the least valuable attacker to a square
int get_smallest_attacker(Bitboard attackers, int side, Bitboard bitboards[]) {

    int smallest_value = 12000;
    int smallest_piece = -1;
    int smallest_square = -1;

    int start = (side == WHITE) ? P : p;
    int end = (side == WHITE) ? K : k;

        Bitboard bitboard = attackers;
        while (bitboard) {
            int square = get_least_sig_bit_index(bitboard);
            int piece = get_piece_at_square(square, bitboards);

            // Make sure the piece is the correct color
            if (side == BLACK && piece < p) {
                POP_BIT(bitboard, square);
                continue;
            } else if (side == WHITE && piece > K) {
                POP_BIT(bitboard, square);
                continue;
            }

            if (piece == -1) {
                POP_BIT(bitboard, square);
                continue;
            }

            int value = MATERIAL_SCORE[piece % 6];

            if (value < smallest_value) {
                smallest_value = value;
                smallest_piece = piece;
                smallest_square = square;
            }

            bitboard &= bitboard - 1;
        }
    return smallest_square;
}

// Check for 3-fold repetition
// Note we actually only check for a single repetition because if the engine chooses a repetition node,
// it will continue to repeat moves under the assumption it's the "best" move.
// As a result, this also cuts down on nodes. 
int is_repetition(Board* board) {
    for (int i = 0; i < board->repetition_index; i++) {
        if (board->repetition_table[i] == board->hash_key) {
            return 1;
        }
    }
    return 0;
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

void pv_scoring(Moves *move_list, Search *search) {
    search->follow_pv = 0;
    for (int i = 0; i < move_list->count; i++) {
        if (search->pv_table[0][search->ply] == move_list->moves[i]) {
            search->score_pv = 1;
            search->follow_pv = 1;
        }
    }
}

int score_move(int move, Search *search) {
    if (search->score_pv) {
        if (search->pv_table[0][search->ply] == move) {
            search->score_pv = 0;
            return 20000;
        }
    }

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

    int start = (search->board->side == WHITE) ? p : P;
    int end = (search->board->side == WHITE) ? k : K;
    int target = MOVE_TARGET(move);

    for (int piece = start; piece <= end; piece++) {
        if (GET_BIT(search->board->bitboards[piece], target)) {
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

/**
 * 1. PV move
 * 2. Captures with MVV/LVA
 * 3. Killer Heuristic
 * 4. History Heuristic
 * 5. Remaining moves
 */
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

// Whether LMR can occur. See https://www.chessprogramming.org/Late_Move_Reductions
int can_reduce(int is_check, int move) {
    return !is_check &&  !MOVE_CAPTURE(move) && !MOVE_PROMOTED(move);
}
