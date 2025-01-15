#include "eval.h"
#include "bitboard.h"
#include "move.h"

#define MIRROR(square) ((square) ^ 56)

/***** Material *****
 * Piece values were calculated to satisfy the following rules:
 * 
 * 3 B/N < 1Q
 * 2 B/N > 1R (but only slightly)
 * B > N (but only slightly)
 * 2B > 2N (bishop pair is better)
 * 3P < B/N
 * Q + P > 2R.
 * 
 * The rule Q+P > 2R is debatable. Usual the rule of thumb is Q+P = 2R.
 * On the other hand, applying the above rule of thumb, that would make
 * - 2B + N > Q and
 * - R + B/N + P > Q
 * 
 * This is usually not the case. The queen is generally preferred in these scenarios.
 * 
 * While some engines choose to follow the former rule of thumb, other choose the latter.
 * Scoring both minors >300 also satisfies the rule that a minor is generally better than 3 pawns.
 *  
 * Thoth will stick to the latter for now because it satisfies the previously mentioned rules regarding queen vs minor value.
 * That scenario is more likely to throw off an engine during game play.
 * 
 * As the engine evolves, tweaks may be needed to consider other evaluation metrics.
 * 
********************/
// int material_score[12] = {
//     100,      // P
//     315,      // N 
//     325,      // B 
//     515,      // R
//    1000,      // Q
//   10000,      // K
//    -100,      // p
//    -315,      // k 
//    -325,      // b 
//    -515,      // r
//   -1000,      // q
//  -10000,      // k
// };

// /***** Position *****
//  * These tables define bonuses (or penalties) for piece position.
//  * The goal is to encourage all the pieces to be on the best squares 
//  * that (generally) give them the most mobility/attacks.
//  * It also strong discourages poor placement of pieces. 
// ********************/

// /***** Pawns *****
//   1. Center pawns should be moved forward in the opening. 
//   2. A small bonus for being in front of the castling squares, and a penalty if they're moved.
//      with the exception of the rook pawns, which gain no bonus mor incur penalty.
//   3. Bonus for centralized pawns.
//   4. Pawns that reach the 5th rank get a bonus. This bonus grows as they advance forward to the 7th. 
// *****************/
// const int POSITION_PAWN[64] = {
//      0,  0,  0,  0,  0,  0,  0,  0,
//     50, 50, 50, 50, 50, 50, 50, 50,
//     10, 10, 20, 30, 30, 20, 10, 10,
//      5,  5, 10, 25, 25, 10,  5,  5,
//      0,  0,  0, 20, 20,  0,  0,  0,
//      5, -5,-10,  0,  0,-10, -5,  5,
//      5, 10, 10,-20,-20, 10, 10,  5,
//      0,  0,  0,  0,  0,  0,  0,  0
// };

// /***** Knights *****
//   The more centralized a knight is, the more mobility is has. 
//   These scores are based off the mobility a knight has on a given square,
//   also taking into account the strength of the squares that it can move to.
// *******************/
// const int POSITION_KNIGHT[64] = {
//     -50,-40,-30,-30,-30,-30,-40,-50,
//     -40,-20,  0,  0,  0,  0,-20,-40,
//     -30,  0, 10, 15, 15, 10,  0,-30,
//     -30,  5, 15, 20, 20, 15,  5,-30,
//     -30,  0, 15, 20, 20, 15,  0,-30,
//     -30,  5, 10, 15, 15, 10,  5,-30,
//     -40,-20,  0,  5,  5,  0,-20,-40,
//     -50,-40,-30,-30,-30,-30,-40,-50,
// };

// /***** Bishops *****
//   1. The more centralized a bishop is, the better.
//   2. Bishops on the long diagonal on their side of the board get a small boost.
//   3. Strenth of squares and material should be considered when calculating trades: 
//      Is a bishop on a square better than a knight on another square?
// *******************/
// const int POSITION_BISHOP[64] = {
//     -20,-10,-10,-10,-10,-10,-10,-20,
//     -10,  0,  0,  0,  0,  0,  0,-10,
//     -10,  0,  5, 10, 10,  5,  0,-10,
//     -10,  5,  5, 10, 10,  5,  5,-10,
//     -10,  0, 10, 10, 10, 10,  0,-10,
//     -10, 10, 10, 10, 10, 10, 10,-10,
//     -10,  5,  0,  0,  0,  0,  5,-10,
//     -20,-10,-10,-10,-10,-10,-10,-20,
// };

// /***** Rooks *****
//   1. A centralized rook is better than a rook on the edge.
//   2. Rooks in the center aren't always strong, especially in the middle game.
//      So, a centralized rook should not influence evaluation in a positive way.
//   3. Squares where the rook can occupy the center files from the 1st rank, 
//      are better than other squares on the 1st rank.
//   4. Rooks on the 7th rank are strong.
// *****************/
// const int POSITION_ROOK[64] = {
//      0,  0,  0,  0,  0,  0,  0,  0,
//      5, 10, 10, 10, 10, 10, 10,  5,
//     -5,  0,  0,  0,  0,  0,  0, -5,
//     -5,  0,  0,  0,  0,  0,  0, -5,
//     -5,  0,  0,  0,  0,  0,  0, -5,
//     -5,  0,  0,  0,  0,  0,  0, -5,
//     -5,  0,  0,  0,  0,  0,  0, -5,
//      0,  0,  0,  5,  5,  0,  0,  0
// };

// /***** Queens *****
//   1. The more centralized a queen is, the better.
//   2. b3 and c2 are good squares in the early stages of the game.
//   3. Since the queen is so mobile, only a small bonus is given for all non-negative squares.
//      As long as the queen is not on the edge, it's placement should not influence the evaluation too much.
// ******************/
const int POSITION_QUEEN[64] = {
    -20,-10,-10, -5, -5,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5,  5,  5,  5,  0,-10,
     -5,  0,  5,  5,  5,  5,  0, -5,
      0,  0,  5,  5,  5,  5,  0, -5,
    -10,  5,  5,  5,  5,  5,  0,-10,
    -10,  0,  5,  0,  0,  0,  0,-10,
    -20,-10,-10, -5, -5,-10,-10,-20
};

// /***** Kings - Middle Game *****
//   1. Kings should be castled and remain behind pawns.
//   2. The further a king wanders up the board, the more dangerous the position is.
// *******************************/
// const int POSITION_KING[64] = {
//     -30,-40,-40,-50,-50,-40,-40,-30,
//     -30,-40,-40,-50,-50,-40,-40,-30,
//     -30,-40,-40,-50,-50,-40,-40,-30,
//     -30,-40,-40,-50,-50,-40,-40,-30,
//     -20,-30,-30,-40,-40,-30,-30,-20,
//     -10,-20,-20,-20,-20,-20,-20,-10,
//      20, 20,  0,  0,  0,  0, 20, 20,
//      20, 30, 10,  0,  0, 10, 30, 20
// };

// /***** Kings - End Game *****
//   1. Centralized kings in the endgame are stronger. 
//   2. The king should try to advance up the board in the endgame
//      vs remaining on their side of the board.
// ****************************/
// const int POSITION_KING_ENDGAME[64] = {
//     -50,-40,-30,-20,-20,-30,-40,-50,
//     -30,-20,-10,  0,  0,-10,-20,-30,
//     -30,-10, 20, 30, 30, 20,-10,-30,
//     -30,-10, 30, 40, 40, 30,-10,-30,
//     -30,-10, 30, 40, 40, 30,-10,-30,
//     -30,-10, 20, 30, 30, 20,-10,-30,
//     -30,-30,  0,  0,  0,  0,-30,-30,
//     -50,-30,-30,-30,-30,-30,-30,-50
// };

const int POSITION_PAWN[64] = 
{
    90,  90,  90,  90,  90,  90,  90,  90,
    30,  30,  30,  40,  40,  30,  30,  30,
    20,  20,  20,  30,  30,  30,  20,  20,
    10,  10,  10,  20,  20,  10,  10,  10,
     5,   5,  10,  20,  20,   5,   5,   5,
     0,   0,   0,   5,   5,   0,   0,   0,
     0,   0,   0, -10, -10,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

// knight positional score
const int POSITION_KNIGHT[64] = 
{
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,  10,  10,   0,   0,  -5,
    -5,   5,  20,  20,  20,  20,   5,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,   5,  20,  10,  10,  20,   5,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5, -10,   0,   0,   0,   0, -10,  -5
};

// bishop positional score
const int POSITION_BISHOP[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,  10,  10,   0,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,  10,   0,   0,   0,   0,  10,   0,
     0,  30,   0,   0,   0,   0,  30,   0,
     0,   0, -10,   0,   0, -10,   0,   0
};

// rook positional score
const int POSITION_ROOK[64] =
{
    50,  50,  50,  50,  50,  50,  50,  50,
    50,  50,  50,  50,  50,  50,  50,  50,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,   0,  20,  20,   0,   0,   0

};

// king positional score
const int POSITION_KING[64] = 
{
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   5,   5,   5,   5,   0,   0,
     0,   5,   5,  10,  10,   5,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   0,   5,  10,  10,   5,   0,   0,
     0,   5,   5,  -5,  -5,   0,   5,   0,
     0,   0,   5,   0, -15,   0,  10,   0
};

Bitboard file_masks[64];
Bitboard rank_masks[64];
Bitboard isolated_masks[64];
Bitboard white_passed_masks[64];
Bitboard black_passed_masks[64];

Bitboard set_masks(int file, int rank) {
    Bitboard mask = 0ULL;

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            int square = SQUARE_INDEX(r, f);
            if (file != -1 && f == file) {
                mask |= SET_BIT(mask, square);
            } 
            if (rank != -1 && r == rank) {
                mask |= SET_BIT(mask, square);
            }
        }
    }
    return mask;
}

void init_evaluation_masks() {
    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            int square = SQUARE_INDEX(r, f);
            file_masks[square] |= set_masks(f, -1);
            rank_masks[square] |= set_masks(-1, r);

            isolated_masks[square] |= set_masks(f-1, -1);
            isolated_masks[square] |= set_masks(f+1, -1);

            white_passed_masks[square] |= set_masks(f - 1, -1);
            white_passed_masks[square] |= set_masks(f, -1);
            white_passed_masks[square] |= set_masks(f + 1, -1);

            black_passed_masks[square] |= set_masks(f - 1, -1);
            black_passed_masks[square] |= set_masks(f, -1);
            black_passed_masks[square] |= set_masks(f + 1, -1);
        }
    }

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            int square = SQUARE_INDEX(r, f);
            for (int i = 0; i < (8 - r); i++) {
                white_passed_masks[square] &= ~rank_masks[(7 - i) * 8 + f]; 
            }
            for (int i = 0; i < r + 1; i++) {
                black_passed_masks[square] &= ~rank_masks[i * 8 + f];
            }
        }
    }
}

int material_score[12] = {
    100,      // white pawn score
    300,      // white knight scrore
    350,      // white bishop score
    500,      // white rook score
   1000,      // white queen score
  10000,      // white king score
   -100,      // black pawn score
   -300,      // black knight scrore
   -350,      // black bishop score
   -500,      // black rook score
  -1000,      // black queen score
 -10000,      // black king score
};

const int square_to_rank[64] = {
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0
};

const int double_pawn_penalty = -10;
const int isolated_pawn_penalty = -10;
const int passed_pawn_bonus[8] = { 0, 10, 30, 50, 75, 100, 150, 200 }; 

const int half_open_file_score = 10;
const int open_file_score = 15;

int evaluate(Board *board) {
    int score = 0;
    int double_pawns;
    for (int piece = P; piece <= k; piece++) {
        Bitboard bitboard = board->bitboards[piece];
        while (bitboard) {
            int square = get_least_sig_bit_index(bitboard);
            
            // Material
            score += material_score[piece];

            // Position
            switch (piece) {
                case P:
                    score += POSITION_PAWN[square]; 
                    double_pawns = count_bits(board->bitboards[P] & file_masks[square]);
                    if (double_pawns > 1) {
                        score += double_pawns * double_pawn_penalty;
                    }
                    if ((board->bitboards[P] & isolated_masks[square]) == 0) {
                        score += isolated_pawn_penalty;
                    }
                    if ((white_passed_masks[square] & board->bitboards[p]) == 0) {
                        score += passed_pawn_bonus[square_to_rank[square]];
                    }
                    break;
                case N: score += POSITION_KNIGHT[square]; break;
                case B: score += POSITION_BISHOP[square]; break;
                case R:
                    // Bonus for rooks on open files
                    if ((board->bitboards[P] & file_masks[square]) == 0) {
                       score += half_open_file_score;
                    }
                    if (((board->bitboards[P] | board->bitboards[p]) & file_masks[square]) == 0) {
                       score += open_file_score;
                    }
                    score += POSITION_ROOK[square]; 
                    break;
                // case Q: score += POSITION_QUEEN[square]; break;
                case K:
                    // Penalty for kings on exposed files
                    if ((board->bitboards[P] & file_masks[square]) == 0) {
                        score -= half_open_file_score;
                    }
                    if (((board->bitboards[P] | board->bitboards[p]) & file_masks[square]) == 0) {
                       score -= open_file_score;
                    } 
                    score += POSITION_KING[square]; 
                    break;
                case p: 
                    score -= POSITION_PAWN[MIRROR(square)]; 
                    double_pawns = count_bits(board->bitboards[p] & file_masks[square]);
                    if (double_pawns > 1) {
                        score -= double_pawns * double_pawn_penalty;
                    }
                    
                    if ((board->bitboards[p] & isolated_masks[square]) == 0) {
                        score -= isolated_pawn_penalty;
                    }
                    
                    if ((black_passed_masks[square] & board->bitboards[P]) == 0) {
                        score -= passed_pawn_bonus[square_to_rank[MIRROR(square)]];
                    }
                    break;
                case n: score -= POSITION_KNIGHT[MIRROR(square)]; break;
                case b: score -= POSITION_BISHOP[MIRROR(square)]; break;
                case r:
                    // Bonus for rooks on open files
                    if ((board->bitboards[p] & file_masks[square]) == 0) {
                       score -= half_open_file_score;
                    }
                    if (((board->bitboards[P] | board->bitboards[p]) & file_masks[square]) == 0) {
                       score -= open_file_score;
                    } 
                    score -= POSITION_ROOK[MIRROR(square)]; 
                    break;
                // case q: score -= POSITION_QUEEN[MIRROR(square)]; break;
                case k:
                // Penalty for kings on exposed files
                    if ((board->bitboards[p] & file_masks[square]) == 0) {
                        score += half_open_file_score;
                     }
                    if (((board->bitboards[P] | board->bitboards[p]) & file_masks[square]) == 0) {
                       score += open_file_score;
                    }  
                    score -= POSITION_KING[MIRROR(square)];
                    break;
            }
            POP_BIT(bitboard, square);
        }
    }
    // score += evaluate_mobility();
    return (board->side == WHITE) ? score : -score;
}

// int evaluate_mobility() {
//     int score = 0;

//     COPY_BOARD();
//     side = WHITE;
//     Moves move_list[1];
//     generate_moves(move_list);  // Generate all moves for the side
//     // Evaluate mobility for white pieces
//     for (int piece = P; piece <= K; piece++) {
//         if (bitboards[piece]) { // Check if there are any pieces of this type on the board
//             int piece_count = 0;  // Initialize a counter for this piece type
//             // Count mobility for this piece type
//             for (int i = 0; i < move_list->count; i++) {
//                 if (MOVE_PIECE(move_list->moves[i]) == piece) {
//                     piece_count++;
//                 }
//             }
//             score += piece_count * mobility_weight(piece);  // Weight mobility
//         }
//     }
    
//     side = BLACK;
//     memset(move_list->moves, 0, sizeof(move_list->moves));
//     move_list->count = 0;
//     generate_moves(move_list);  // Generate all moves for the side
//     // Evaluate mobility for black pieces
//     for (int piece = p; piece <= k; piece++) {
//         if (bitboards[piece]) { // Check if there are any pieces of this type on the board
//             int piece_count = 0;  // Initialize a counter for this piece type

//             // Count mobility for this piece type
//             for (int i = 0; i < move_list->count; i++) {
//                 if (MOVE_PIECE(move_list->moves[i]) == piece) {
//                     piece_count++;
//                 }
//             }
//             score -= piece_count * mobility_weight(piece);  // Weight mobility
//         }
//     }
//     UNDO();
//     return score;
// }

// // Function to assign weights to different pieces' mobility
// int mobility_weight(int piece) {
//     switch (piece) {
//         case N: case n: return 5;
//         case B: case b: return 5;
//         case R: case r: return 2;
//         case Q: case q: return 1;
//         default: return 0;
//     }
// }