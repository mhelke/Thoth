#include "eval.h"
#include "bitboard.h"

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
int material_score[12] = {
    100,      // P
    315,      // N 
    325,      // B 
    515,      // R
   1000,      // Q
  10000,      // K
   -100,      // p
   -315,      // k 
   -325,      // b 
   -515,      // r
  -1000,      // q
 -10000,      // k
};

/***** Position *****
 * These tables define bonuses (or penalties) for piece position.
 * The goal is to encourage all the pieces to be on the best squares 
 * that (generally) give them the most mobility/attacks.
 * It also strong discourages poor placement of pieces. 
********************/

/***** Pawns *****
  1. Center pawns should be moved forward in the opening. 
  2. A small bonus for being in front of the castling squares, and a penalty if they're moved.
     with the exception of the rook pawns, which gain no bonus mor incur penalty.
  3. Bonus for centralized pawns.
  4. Pawns that reach the 5th rank get a bonus. This bonus grows as they advance forward to the 7th. 
*****************/
const int POSITION_PAWN[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
     5,  5, 10, 25, 25, 10,  5,  5,
     0,  0,  0, 20, 20,  0,  0,  0,
     5, -5,-10,  0,  0,-10, -5,  5,
     5, 10, 10,-20,-20, 10, 10,  5,
     0,  0,  0,  0,  0,  0,  0,  0
};

/***** Knights *****
  The more centralized a knight is, the more mobility is has. 
  These scores are based off the mobility a knight has on a given square,
  also taking into account the strength of the squares that it can move to.
*******************/
const int POSITION_KNIGHT[64] = {
    -50,-40,-30,-30,-30,-30,-40,-50,
    -40,-20,  0,  0,  0,  0,-20,-40,
    -30,  0, 10, 15, 15, 10,  0,-30,
    -30,  5, 15, 20, 20, 15,  5,-30,
    -30,  0, 15, 20, 20, 15,  0,-30,
    -30,  5, 10, 15, 15, 10,  5,-30,
    -40,-20,  0,  5,  5,  0,-20,-40,
    -50,-40,-30,-30,-30,-30,-40,-50,
};

/***** Bishops *****
  1. The more centralized a bishop is, the better.
  2. Bishops on the long diagonal on their side of the board get a small boost.
  3. Strenth of squares and material should be considered when calculating trades: 
     Is a bishop on a square better than a knight on another square?
*******************/
const int POSITION_BISHOP[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 10, 10,  5,  5,-10,
    -10,  0, 10, 10, 10, 10,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
};

/***** Rooks *****
  1. A centralized rook is better than a rook on the edge.
  2. Rooks in the center aren't always strong, especially in the middle game.
     So, a centralized rook should not influence evaluation in a positive way.
  3. Squares where the rook can occupy the center files from the 1st rank, 
     are better than other squares on the 1st rank.
  4. Rooks on the 7th rank are strong.
*****************/
const int POSITION_ROOK[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,
     5, 10, 10, 10, 10, 10, 10,  5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     0,  0,  0,  5,  5,  0,  0,  0
};

/***** Queens *****
  1. The more centralized a queen is, the better.
  2. b3 and c2 are good squares in the early stages of the game.
  3. Since the queen is so mobile, only a small bonus is given for all non-negative squares.
     As long as the queen is not on the edge, it's placement should not influence the evaluation too much.
******************/
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

/***** Kings - Middle Game *****
  1. Kings should be castled and remain behind pawns.
  2. The further a king wanders up the board, the more dangerous the position is.
*******************************/
const int POSITION_KING[64] = {
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -30,-40,-40,-50,-50,-40,-40,-30,
    -20,-30,-30,-40,-40,-30,-30,-20,
    -10,-20,-20,-20,-20,-20,-20,-10,
     20, 20,  0,  0,  0,  0, 20, 20,
     20, 30, 10,  0,  0, 10, 30, 20
};

/***** Kings - End Game *****
  1. Centralized kings in the endgame are stronger. 
  2. The king should try to advance up the board in the endgame
     vs remaining on their side of the board.
****************************/
const int POSITION_KING_ENDGAME[64] = {
    -50,-40,-30,-20,-20,-30,-40,-50,
    -30,-20,-10,  0,  0,-10,-20,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 30, 40, 40, 30,-10,-30,
    -30,-10, 20, 30, 30, 20,-10,-30,
    -30,-30,  0,  0,  0,  0,-30,-30,
    -50,-30,-30,-30,-30,-30,-30,-50
};

int evaluate() {
    int score = 0;
    for (int piece = P; piece <= k; piece++) {
        Bitboard bitboard = bitboards[piece];
        while (bitboard) {
            int square = get_least_sig_bit_index(bitboard);
            
            // Material
            score += material_score[piece];

            // Position
            switch (piece) {
                case P: score += POSITION_PAWN[square]; break;
                case N: score += POSITION_KNIGHT[square]; break;
                case B: score += POSITION_BISHOP[square]; break;
                case R: score += POSITION_ROOK[square]; break;
                case Q: score += POSITION_QUEEN[square]; break;
                case K: score += POSITION_KING[square]; break;
                case p: score -= POSITION_PAWN[MIRROR(square)]; break;
                case n: score -= POSITION_KNIGHT[MIRROR(square)]; break;
                case b: score -= POSITION_BISHOP[MIRROR(square)]; break;
                case r: score -= POSITION_ROOK[MIRROR(square)]; break;
                case q: score -= POSITION_QUEEN[MIRROR(square)]; break;
                case k: score -= POSITION_KING[MIRROR(square)]; break;
            }
            pop_bit(bitboard, square);
        }
    }
    return (side == WHITE) ? score : -score;
}