#include "eval.h"
#include "bitboard.h"

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

int evaluate() {
    int score = 0;
    for (int i = P; i <= k; i++) {
        Bitboard bitboard = bitboards[i];
        while (bitboard) {
            int square = get_least_sig_bit_index(bitboard);
            score += material_score[i];
            pop_bit(bitboard, square);
        }
    }
    return (side == WHITE) ? score : -score;
}


