#include <stdio.h>

#include "eval.h"
#include "bitboard.h"
#include "move.h"

#define MIRROR(square) ((square) ^ 56)
#define FILE_ABC_MASK 0x0707070707070707ULL
#define FILE_FGH_MASK 0xE0E0E0E0E0E0E0E0ULL


// TODO: Might be weighting different things too much or too little now!
// TODO: Also, check if the PST values should be changed back to the larger values? 
struct {
    int phase;
    int material[2];
    int openingPST[2];
    int endgamePST[2];
    int openingMobility[2];
    int endgameMobility[2];
    int pawnStructure[2];
    int materialAdj[2];
    int kingSafety[2];
    int positionMetrics[2];
} Score;

/***** Position *****
 * These tables define bonuses (or penalties) for piece position.
 * The goal is to encourage all the pieces to be on the best squares 
 * that (generally) give them the most mobility/attacks.
 * It also strong discourages poor placement of pieces. 
********************/

/***** Pawns - Opening *****
  1. Center pawns should be moved forward in the opening. 
  2. A small bonus for being in front of the castling squares, and a penalty if they're moved.
     with the exception of the g-pawn which can be fianchettoed
  3. Bonus for centralized pawns.
  4. Pawns that reach the 5th rank get a bonus. This bonus grows as they advance forward to the 7th. 
*****************/
static const int PAWN_OPENING_POSITION[64] = 
{
    0,   0,   0,   0,   0,   0,  0,   0,
    98, 134,  61,  95,  68, 126, 34, -11,
    -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,   5,  12,  17,   5, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
    0,   0,   0,   0,   0,   0,  0,   0,
};

/***** Pawns - Endgame *****
  1. The further down the board a pawn is, the better.
  2. Small bonuses/penalties for pawns below the 5th rank.
     This helps adjust the values in the middle game.
*****************/
static const int PAWN_ENDGAME_POSITION[64] = 
{
    0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
    94, 100,  85,  67,  56,  53,  82,  84,
    32,  24,  13,   5,  -2,   4,  17,  17,
    13,   9,  -3,  -7,  -7,  -8,   3,  -1,
    4,   7,  -6,   1,   0,  -5,  -1,  -8,
    13,   8,   8,  10,  13,   0,   2,  -7,
    0,   0,   0,   0,   0,   0,   0,   0,
};

/***** Knights - Opening *****
  1. The more centralized a knight is, the more mobility is has. 
     These scores are based off the mobility a knight has on a given square,
     also taking into account the strength of the squares that it can move to.
  2. Knights past the 5th rank are very powerful because they can cramp the 
     opponent's position and attack their pieces better.
*******************/
static const int KNIGHT_OPENING_POSITION[64] = 
{
-167, -89, -34, -49,  61, -97, -15, -107,
    -73, -41,  72,  36,  23,  62,   7,  -17,
    -47,  60,  37,  65,  84, 129,  73,   44,
    -9,  17,  19,  53,  37,  69,  18,   22,
    -13,   4,  16,  13,  28,  19,  21,   -8,
    -23,  -9,  12,  10,  19,  17,  25,  -16,
    -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
};

/***** Knights - Endgame *****
    The main principle in the endgame is mobility.
    Knights should aim for maximum mobility.
*******************/
static const int KNIGHT_ENDGAME_POSITION[64] = 
{
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
};

/***** Bishops - Opening *****
     Since the endgame has fewer pieces, a more centralized knight is better.
     This is the only metric that can be used during this phase of the game given
     the importance of mobility.
*******************/
static const int BISHOP_OPENING_POSITION[64] = 
{
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
    -4,   5,  19,  50,  37,  37,   7,  -2,
    -6,  13,  13,  26,  34,  12,  10,   4,
    0,  15,  15,  15,  14,  27,  18,  10,
    4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
    
};

/***** Bishops - Endgame *****
    The main principle in the endgame is mobility.
    Bishops should aim for maximum mobility. 
*******************/
static const int BISHOP_ENDGAME_POSITION[64] = 
{
    -14, -21, -11,  -8, -7,  -9, -17, -24,
    -8,  -4,   7, -12, -3, -13,  -4, -14,
    2,  -8,   0,  -1, -2,   6,   0,   4,
    -3,   9,  12,   9, 14,  10,   3,   2,
    -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17
};

/***** Rooks - Opening *****
  1. A centralized rook is better than a rook on the edge.
  2. Rooks in the center aren't always strong in the opening and middle game phases.
     So, a centralized rook should not influence evaluation significantly.
  3. Squares where the rook can occupy the center files from the 1st rank, 
     are better than other squares on the 1st rank. (f-file gets a small boost to encourage castling)
  4. Rooks on the 7th rank are strong.
*****************/
static const int ROOK_OPENING_POSITION[64] =
{
    32,  42,  32,  51, 63,  9,  31,  43,
    27,  32,  58,  62, 80, 67,  26,  44,
    -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,

};

/***** Rooks - Endgame *****
  1. Rooks should continue to be centralized
  2. Typically, the queenside ends up being where rooks go in the endgame (due to castling kingside)
     These scores try to account for that very slightly. This especially helps when scoring the middle game.
  3. The further up the board, the better due to cutting off paths of the king and other pawns.
     Also, a rook should be behind the opponent's passed pawns. 
*****************/
static const int ROOK_ENDGAME_POSITION[64] =
{
  5,   5,   5,   5,   5,   5,   5,   5,
  20,  20,  20,  20,  20,  20,  20,  20,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  0,   0,   0,   2,   2,   0,   0,   0

};

/***** Queens - Opening *****
  1. The queen should restrain from coming out too early to avoid being chased away with tempo.
  2. b3, c2, d1, d2, e2, f3, g4, h5 are all typical first moves for the queen in many openings.
  3. Since the queen is so mobile, only a small bonus is given for all non-negative squares.
     As long as the queen is not on the edge, it's placement should not influence the evaluation too much.
     However, a queen that can infiltrate the kingside is very strong. This bonus will be contradicted by other factors
     if the position is not as good as it seems.
******************/
static const int QUEEN_OPENING_POSITION[64] = 
{
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
    -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
    -1, -18,  -9,  10, -15, -25, -31, -50,
};

/***** Queens - Endgame *****
  1. The queen should be centralized to maximize mobility.
  2. The further up the board the queen is, the better due
     to helping pawns advance, cutting off the king, etc.
******************/
static const int QUEEN_ENDGAME_POSITION[64] =
{
    -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
    3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
};

/***** Kings - Opening *****
  1. Kings should be castled and remain behind pawns, but given the ability
     if needed, to advance a single square up for protection. This however results 
     in a smaller bonus since a king on the back rank is typically safer.
  2. The king is discouraged from all squares that aren't in the castling area
     However, remaining on e8 doesn't get a penalty, but a smaller bonus.
  3. Castling kingside is generally better than queenside, hence the extra bonus. 
  4. The further a king wanders up the board, the more dangerous the position is.
*******************************/
static const int KING_OPENING_POSITION[64] = 
{
    -65,  23,  16, -15, -56, -34,   2,  13,
    29,  -1, -20,  -7,  -8,  -4, -38, -29,
    -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
    1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

/***** Kings - Endgame *****
  1. Centralized kings in the endgame are stronger due to
     only being able to move a single square and the 
     lack of pieces on the board. 
  2. The king should try to advance up the board in the endgame
     vs remaining on their side of the board.
****************************/
static const int KING_ENDGAME_POSITION[64] =
{
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
    10,  17,  23,  15,  20,  45,  44,  13,
    -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};

/***** Material *****
 * Material value differs between the game phases.
 * For example, a pawn in the endgame is significantly more valuable
 * than a pawn in the opening. Minor pieces should be considered more valuable during the
 * opening and become less valuable as the game advances.
 * 
 * Piece values were calculated to satisfy the following rules:
 * 
 * 3 B/N < 1Q in the endgame, but the opposite in the opening (only slightly)
 * 2 B/N > 1R
 * B > N (but only slightly)
 * 2B > 2N (bishop pair is better)
 * 3P < B/N
 * Q + P > 2R (advantage is smaller in the endgame)
 * 
 * The rule Q+P > 2R is debatable. Usual the rule of thumb is Q+P = 2R.
 * On the other hand, applying the above rule of thumb, that would make
 * - 2B + N > Q and
 * - R + B/N + P > Q
 * 
 * This is usually not the case. The queen is generally preferred in these scenarios.
 * 
 * While some engines choose to follow the former rule of thumb, other choose the latter.
 * Thoth's scoring also enforces the general rule that a minor is (usually) better than 3 pawns.
 *  
 * Thoth will stick to the latter for now because it satisfies the previously mentioned rules regarding queen vs minor value.
 * Of course, in all these situations, material is not the only metric to consider, and material should not be the only influence 
 * on the engine's decisions, but it should play a significant role. 
 * 
 * As the engine evolves, tweaks may be needed to consider other evaluation metrics.
 * 
********************/

enum {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

// Material scores adjusted based on the number of own pawns on the board.
// Rooks are better as pawns are traded off, while knights are worse.
static const int KNIGHT_ADJ[9] = { -20, -16, -12, -8, -4,  0,  4,  8, 12 };
static const int ROOK_ADJ[9] = { 15,  12,   9,  6,  3,  0, -3, -6, -9 };

static const int DOUBLE_PAWN_PENALTY = -20;
static const int ISOLATED_PAWN_PENALTY = -15;

// Passed pawn scores indexed by the rank.
static const int PASSED_PAWN_BONUS[8] = { 0, 10, 20, 40, 60, 80, 100, 200 }; 

static const int HALF_OPEN_FILE_SCORE = 5;
static const int OPEN_FILE_SCORE = 10;

static const int KING_SAFETY_BONUS = 10;
static const int BISHOP_PAIR_BONUS = 30;
static const int KNIGHT_PAIR_BONUS = 8;
static const int KNIGHT_PAIR_PENALTY = -8;
static const int BISHOP_ENDGAME_BONUS = 10;
static const int KNIGHT_BLOCK_C3_PENALTY = -5;
static const int QUEEN_DEVELOPMENT_PENALTY = -2;
static const int TEMPO_BONUS = 5;

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

static const int square_to_rank[64] = {
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 0, 0, 0, 0, 0, 0
};

// Returns the last evaluated material score for the given side.
// If both sides have 0 material and the phase is > 0, then the material has not been calculated yet.
// The caller should ensure that material has been calculated before calling this function.
int get_material(int side) {
    if (Score.material[WHITE] == 0 && Score.material[BLACK] == 0 && Score.phase > 0) {
        printf("    [ERROR] Material has not been calculated yet!\n");
        return 0; // Return 0 or some default value indicating an error
    }
    return Score.material[side];
}

int evaluate(Board *board) {
    // Clear the scores
    Score.phase = 0;
    Score.material[WHITE] = 0;
    Score.material[BLACK] = 0;
    Score.openingPST[WHITE] = 0;
    Score.openingPST[BLACK] = 0;
    Score.endgamePST[WHITE] = 0;
    Score.endgamePST[BLACK] = 0;
    Score.openingMobility[WHITE] = 0;
    Score.openingMobility[BLACK] = 0;
    Score.endgameMobility[WHITE] = 0;
    Score.endgameMobility[BLACK] = 0;
    Score.pawnStructure[WHITE] = 0;
    Score.pawnStructure[BLACK] = 0;
    Score.materialAdj[WHITE] = 0;
    Score.materialAdj[BLACK] = 0;
    Score.kingSafety[WHITE] = 0;
    Score.kingSafety[BLACK] = 0;
    Score.positionMetrics[WHITE] = 0;
    Score.positionMetrics[BLACK] = 0;

    // Keep track of mobility
    int wBishopMob = 0;
    int bBishopMob = 0;
    int wKnightMob = 0;
    int bKnightMob = 0;
    int wRookMob = 0;
    int bRookMob = 0;
    int wQueenMob = 0;
    int bQueenMob = 0;

    int wPawns = count_bits(board->bitboards[P]);
    int bPawns = count_bits(board->bitboards[p]);

    for (int piece = P; piece <= k; piece++) {
        Bitboard bitboard = board->bitboards[piece];
        while (bitboard) {
            int square = get_least_sig_bit_index(bitboard);
            int mirror_square = MIRROR(square);
            int double_pawns = 0;

            Bitboard file_mask = file_masks[square];
            Bitboard white_pawns_on_file = board->bitboards[P] & file_mask;
            Bitboard black_pawns_on_file = board->bitboards[p] & file_mask;
            Bitboard any_pawn_on_file = white_pawns_on_file | black_pawns_on_file;

            // Position
            switch (piece) {
                case P:
                    Score.material[WHITE] += MATERIAL_SCORE[PAWN];
                    Score.openingPST[WHITE] += PAWN_OPENING_POSITION[square];
                    Score.endgamePST[WHITE] += PAWN_ENDGAME_POSITION[square];
                   

                    double_pawns = count_bits(white_pawns_on_file);
                    if (double_pawns > 1) {
                        Score.pawnStructure[WHITE] += double_pawns * DOUBLE_PAWN_PENALTY;
                    }
                    if ((board->bitboards[P] & isolated_masks[square]) == 0) {
                        Score.pawnStructure[WHITE] +=  ISOLATED_PAWN_PENALTY;
                    }
                    if ((white_passed_masks[square] & board->bitboards[p]) == 0) {
                        Score.pawnStructure[WHITE] +=  PASSED_PAWN_BONUS[square_to_rank[square]];
                    }
                    break;
                case N: 
                    Score.phase += 1;
                    Score.material[WHITE] += MATERIAL_SCORE[KNIGHT];
                    Score.materialAdj[WHITE] += KNIGHT_ADJ[wPawns];
                    Score.openingPST[WHITE]  += KNIGHT_OPENING_POSITION[square];
                    Score.endgamePST[WHITE]  += KNIGHT_ENDGAME_POSITION[square];
                    wKnightMob += count_bits(board->knight_attacks[square] & (~board->occupancies[WHITE]));
                    // If there is a pawn on c2 and a knight on c3, the knight gets a penalty of 5 
                    if (square == c3 && (board->bitboards[P] & c2) && (board->bitboards[P] & d4)  && !(board->bitboards[P] & e4)) Score.positionMetrics[WHITE] += KNIGHT_BLOCK_C3_PENALTY;
                    break;
                case B: 
                    Score.phase += 1;
                    Score.material[WHITE] += MATERIAL_SCORE[BISHOP];
                    Score.openingPST[WHITE]  += BISHOP_OPENING_POSITION[square];
                    Score.endgamePST[WHITE]  += BISHOP_ENDGAME_POSITION[square];
                    wBishopMob += count_bits(get_bishop_attacks(square, board->occupancies[BOTH], board));
                    break;
                case R:
                    Score.phase += 2;
                    Score.material[WHITE] += MATERIAL_SCORE[ROOK];
                    Score.materialAdj[WHITE] += ROOK_ADJ[wPawns];
                    Score.openingPST[WHITE]  += ROOK_OPENING_POSITION[square];
                    Score.endgamePST[WHITE]  += ROOK_ENDGAME_POSITION[square];

                    // Bonus for rooks on open files
                    if (white_pawns_on_file == 0) {
                       Score.positionMetrics[WHITE] += HALF_OPEN_FILE_SCORE;
                    }
                    if (any_pawn_on_file == 0) {
                       Score.positionMetrics[WHITE] += OPEN_FILE_SCORE;
                    }
                    wRookMob += count_bits(get_rook_attacks(square, board->occupancies[BOTH], board));
                    break;
                case Q:
                    Score.phase += 4;
                    Score.material[WHITE] += MATERIAL_SCORE[QUEEN];

                    Score.openingPST[WHITE]  += QUEEN_OPENING_POSITION[square];
                    Score.endgamePST[WHITE]  += QUEEN_ENDGAME_POSITION[square];
                    wQueenMob += count_bits(get_queen_attacks(square, board->occupancies[BOTH], board));
                    // Prevent the queen from developing too early
                    if (rank_masks[square] > 2) {
                        if (board->bitboards[N] & b1) Score.positionMetrics[WHITE] += QUEEN_DEVELOPMENT_PENALTY; 
                        if (board->bitboards[N] & g1) Score.positionMetrics[WHITE] += QUEEN_DEVELOPMENT_PENALTY; 
                        if (board->bitboards[B] & c1) Score.positionMetrics[WHITE] += QUEEN_DEVELOPMENT_PENALTY; 
                        if (board->bitboards[B] & f1) Score.positionMetrics[WHITE] += QUEEN_DEVELOPMENT_PENALTY;
                    }
                    break;
                case K:
                    Score.material[WHITE] += MATERIAL_SCORE[KING];
                    Score.openingPST[WHITE]  += KING_OPENING_POSITION[square];
                    Score.endgamePST[WHITE]  += KING_ENDGAME_POSITION[square];
                    // Penalty for kings on exposed files
                    if (white_pawns_on_file == 0) {
                        Score.kingSafety[WHITE] -= HALF_OPEN_FILE_SCORE;
                    }
                    if (any_pawn_on_file == 0) {
                        Score.kingSafety[WHITE] -= OPEN_FILE_SCORE;
                    } 
                    // Pieces in front of king protecting it
                    Score.kingSafety[WHITE] += count_bits(board->king_attacks[square] & board->occupancies[WHITE]) * KING_SAFETY_BONUS;
                    break;
                case p:
                    Score.material[BLACK] += MATERIAL_SCORE[PAWN];
                    Score.openingPST[BLACK] += PAWN_OPENING_POSITION[mirror_square];
                    Score.endgamePST[BLACK] += PAWN_ENDGAME_POSITION[mirror_square];
                    double_pawns = count_bits(black_pawns_on_file);
                    if (double_pawns > 1) {
                        Score.pawnStructure[BLACK] += double_pawns * DOUBLE_PAWN_PENALTY;
                    }
                    
                    if ((board->bitboards[p] & isolated_masks[square]) == 0) {
                        Score.pawnStructure[BLACK] += ISOLATED_PAWN_PENALTY;
                    }
                    
                    if ((black_passed_masks[square] & board->bitboards[P]) == 0) {
                        Score.pawnStructure[BLACK] += PASSED_PAWN_BONUS[square_to_rank[mirror_square]];
                    }
                    break;
                case n:
                    Score.phase += 1;
                    Score.material[BLACK] += MATERIAL_SCORE[KNIGHT];
                    Score.materialAdj[BLACK] += KNIGHT_ADJ[bPawns];
                    Score.openingPST[BLACK] += KNIGHT_OPENING_POSITION[mirror_square];
                    Score.endgamePST[BLACK] += KNIGHT_ENDGAME_POSITION[mirror_square];
                    bKnightMob += count_bits(board->knight_attacks[square] & (~board->occupancies[BLACK]));
                    if (square == c6 && (board->bitboards[p] & c7) && (board->bitboards[p] & d5)  && !(board->bitboards[p] & e5)) Score.positionMetrics[WHITE] += KNIGHT_BLOCK_C3_PENALTY;
                    break;
                case b:
                    Score.phase += 1;
                    Score.material[BLACK] += MATERIAL_SCORE[BISHOP];
                    Score.openingPST[BLACK] += BISHOP_OPENING_POSITION[mirror_square]; 
                    Score.endgamePST[BLACK] += BISHOP_ENDGAME_POSITION[mirror_square];
                    bBishopMob += count_bits(get_bishop_attacks(square, board->occupancies[BOTH], board)); 
                    break;
                case r:
                    Score.phase += 2;
                    Score.material[BLACK] += MATERIAL_SCORE[ROOK];
                    Score.materialAdj[BLACK] += ROOK_ADJ[bPawns];
                    Score.openingPST[BLACK] += ROOK_OPENING_POSITION[mirror_square];
                    Score.endgamePST[BLACK] += ROOK_ENDGAME_POSITION[mirror_square];
                    // Bonus for rooks on open files
                    if (black_pawns_on_file == 0) {
                       Score.positionMetrics[BLACK] += HALF_OPEN_FILE_SCORE;
                    }
                    if (any_pawn_on_file == 0) {
                       Score.positionMetrics[BLACK] += OPEN_FILE_SCORE;
                    }
                    bRookMob += count_bits(get_rook_attacks(square, board->occupancies[BOTH], board));
                    break;
                case q:
                    Score.phase += 4;
                    Score.material[BLACK] += MATERIAL_SCORE[QUEEN];
                    Score.openingPST[BLACK] += QUEEN_OPENING_POSITION[mirror_square];
                    Score.endgamePST[BLACK] += QUEEN_ENDGAME_POSITION[mirror_square];
                    bQueenMob += count_bits(get_queen_attacks(square, board->occupancies[BOTH], board));

                    // Prevent the queen from developing too early
                    if (rank_masks[square] < 7) {
                        if (board->bitboards[N] & b8) Score.positionMetrics[BLACK] += QUEEN_DEVELOPMENT_PENALTY; 
                        if (board->bitboards[N] & g8) Score.positionMetrics[BLACK] += QUEEN_DEVELOPMENT_PENALTY; 
                        if (board->bitboards[B] & c8) Score.positionMetrics[BLACK] += QUEEN_DEVELOPMENT_PENALTY; 
                        if (board->bitboards[B] & f8) Score.positionMetrics[BLACK] += QUEEN_DEVELOPMENT_PENALTY;
                    }
                    break;
                case k:
                    Score.material[BLACK] += MATERIAL_SCORE[KING];
                    Score.openingPST[BLACK] += KING_OPENING_POSITION[mirror_square];
                    Score.endgamePST[BLACK] += KING_ENDGAME_POSITION[mirror_square];
                    // Penalty for kings on exposed files
                    if (black_pawns_on_file == 0) {
                        Score.kingSafety[BLACK] -= HALF_OPEN_FILE_SCORE;
                    }
                    if (any_pawn_on_file == 0) {
                        Score.kingSafety[BLACK] -= OPEN_FILE_SCORE;
                    }  
                    // Pieces in front of king protecting it
                    Score.kingSafety[BLACK] += count_bits(board->king_attacks[square] & board->occupancies[BLACK]) * KING_SAFETY_BONUS;
                    break;
            }
            POP_BIT(bitboard, square);
        }
    }

    int pawns = wPawns + bPawns;
    int white_bishops = count_bits(board->bitboards[B]);
    int black_bishops = count_bits(board->bitboards[b]);
    int white_knights = count_bits(board->bitboards[N]);
    int black_knights = count_bits(board->bitboards[n]);
    
    // Bishop pair
    if (white_bishops > 1) Score.positionMetrics[WHITE] += BISHOP_PAIR_BONUS;
    if (black_bishops > 1) Score.positionMetrics[BLACK] += BISHOP_PAIR_BONUS;

    // Knight pair
    if (count_bits(board->bitboards[N]) > 1) Score.positionMetrics[WHITE] += pawns > 10 ? KNIGHT_PAIR_BONUS : -KNIGHT_PAIR_PENALTY;
    if (count_bits(board->bitboards[n]) > 1) Score.positionMetrics[BLACK] += pawns > 10 ? KNIGHT_PAIR_BONUS : -KNIGHT_PAIR_PENALTY;

    /** TODO: This metric only applies in the endgame and should not be weighted as much in the opening or middlegame  **/
    
    // If there are pawns on both sides of the board, bishops are better than knights in the endgame
    if (Score.phase <= 12) { // Only take effect starting in the middlegame
        int pawns_on_abc_files = (board->bitboards[P] & FILE_ABC_MASK) || (board->bitboards[p] & FILE_ABC_MASK);
        int pawns_on_fgh_files = (board->bitboards[P] & FILE_FGH_MASK) || (board->bitboards[p] & FILE_FGH_MASK);

        if (pawns_on_abc_files && pawns_on_fgh_files) {
            if (white_bishops > 0) {
                Score.positionMetrics[WHITE] += BISHOP_ENDGAME_BONUS;
            }
            if (black_bishops > 0) {
                Score.positionMetrics[BLACK] += BISHOP_ENDGAME_BONUS;
            }
        }
    }

    // Mobility adjustments are made in such a way that a score of 0 is roughly "average" mobility for each piece in the given game phase. 
    Score.openingMobility[WHITE] += 4 * (wKnightMob - 4);
    Score.endgameMobility[WHITE] += 4 * (wKnightMob - 4);
    Score.openingMobility[WHITE] += 3 * (wBishopMob - 7);
    Score.endgameMobility[WHITE] += 3 * (wBishopMob - 7);
    Score.openingMobility[WHITE] += 2 * (wRookMob - 7); 
    Score.endgameMobility[WHITE] += 4 * (wRookMob - 7); 
    Score.openingMobility[WHITE] += 1 * (wQueenMob - 14);
    Score.endgameMobility[WHITE] += 2 * (wQueenMob - 14);
    Score.openingMobility[BLACK] += 4 * (bKnightMob - 4);
    Score.endgameMobility[BLACK] += 4 * (bKnightMob - 4);
    Score.openingMobility[BLACK] += 3 * (bBishopMob - 7);
    Score.endgameMobility[BLACK] += 3 * (bBishopMob - 7);
    Score.openingMobility[BLACK] += 2 * (bRookMob - 7); 
    Score.endgameMobility[BLACK] += 4 * (bRookMob - 7); 
    Score.openingMobility[BLACK] += 1 * (bQueenMob - 14);
    Score.endgameMobility[BLACK] += 2 * (bQueenMob - 14);
        
    /* 
        Tapered Evaluation

        Determine what phase of the game it is.
        For opening and endgame phases, the respective material and position scores are used.
        For the middle game, the respective opening and endgame scores are interpolated. The final material and position scores
        are determined by how far along in each phase the game is. This provides a more accurate measure of material and position
        during the middle game. e.g. - A pawn's position in the opening may be best in the center, while the end game is best on the 7th rank.
        However, in the middle game, neither position may be 100% right. As the middle game advances, the scores slowly shift towards the endgame,
        this allows the engine to make more precise decisions.
    */
    if (Score.phase > 24) {
        Score.phase = 24;
    }

    int middle_game_weight = Score.phase;
    int endgame_weight = 24 - middle_game_weight;

    // Final score calculation
    int score = 0;
 
    // Add material, mobility, and PST scores. Interpolate for the middle game.
    // King safety is included in the opening score, but as the game progresses, this metric is reduced until it doesn't matter in the endgame.
    int opening_score = (Score.material[WHITE] + Score.openingPST[WHITE] + Score.openingMobility[WHITE] + Score.kingSafety[WHITE]) 
        - (Score.material[BLACK] + Score.openingPST[BLACK] + Score.openingMobility[BLACK] + Score.kingSafety[BLACK]);
    int endgame_score = (Score.material[WHITE] + Score.endgamePST[WHITE] + Score.endgameMobility[WHITE]) 
        - (Score.material[BLACK] + Score.endgamePST[BLACK] + Score.endgameMobility[BLACK]);

    score += ((opening_score * middle_game_weight) + (endgame_score * endgame_weight)) / 24;
    
    /* 
        Game phase independent scores. These scores keep track of more complex positional evaluations such as pawn structure or positional metrics.
        These values do not need to be interpolated as they are not dependent on the game phase.
    */
    score += (Score.pawnStructure[WHITE] - Score.pawnStructure[BLACK]);
    score += (Score.positionMetrics[WHITE] - Score.positionMetrics[BLACK]);
    score += (Score.materialAdj[WHITE] - Score.materialAdj[BLACK]);

    /*                                                                  
        Account of low material situations. Without this, the engine will think it is leading when it has insufficient material.

        - 1 Minor cannot win                               
        - 2 knights cannot win                      
        - Rook vs Minor is drawish                           
        - Rook + Minor vs Rook is drawish                        
    */
   int leading, trailing = 0;
    if (score > 0) {
        leading = WHITE;
        trailing = BLACK;
    } else {
        leading = BLACK;
        trailing = WHITE;
    }

    int leadingPawns = (leading == WHITE) ? wPawns : bPawns; 
    if (leadingPawns == 0) {

        // Cases where it is impossible to win with no pawns
        if (Score.material[leading] < 400) {
            return 0;
        }
        if (bPawns == 0 && Score.material[trailing] == 2 * MATERIAL_SCORE[N]) {
            return 0;
        }

        // Cases where it is possible, but very unlikely to win with no pawns
        if (Score.material[leading] == MATERIAL_SCORE[ROOK] && Score.material[trailing] == MATERIAL_SCORE[BISHOP]) {
            score /= 2;
        }
        if (Score.material[leading] == MATERIAL_SCORE[ROOK] && Score.material[trailing] == MATERIAL_SCORE[KNIGHT]) {
            score /= 2;
        }
        if (Score.material[leading] == MATERIAL_SCORE[ROOK] + MATERIAL_SCORE[BISHOP] && Score.material[trailing] == MATERIAL_SCORE[ROOK]) {
            score /= 2;
        }
        if (Score.material[leading] == MATERIAL_SCORE[ROOK] + MATERIAL_SCORE[KNIGHT] && Score.material[trailing] == MATERIAL_SCORE[ROOK]) {
            score /= 2;
        }
    }

    // Give the side that has a tempo a small bonus since that side dictates the game flow.
    if (board->side == WHITE) score += TEMPO_BONUS;
    else score -= TEMPO_BONUS;
 
    return (board->side == WHITE) ? score : -score;
}

void printEvalFactor(int wh, int bl) {
  printf("white %4d, black %4d, total: %4d \n", wh, bl, wh - bl);
}

/**
 * Prints all evaluation metrics for debugging purposes.
 */
void printEval(Board *board) {
  printf("------------------------------------------\n");
  printf("Total value (for side to move): %d \n", evaluate(board));
  printf("Material balance:     %d \n", Score.material[WHITE] - Score.material[BLACK]);
  printf("Material:             "); printEvalFactor(Score.material[WHITE], Score.material[BLACK]);
  printf("Material adj:         "); printEvalFactor(Score.materialAdj[WHITE], Score.materialAdj[BLACK]);
  printf("Op PST:               "); printEvalFactor(Score.openingPST[WHITE], Score.openingPST[BLACK]);
  printf("Eg PST:               "); printEvalFactor(Score.endgamePST[WHITE], Score.endgamePST[BLACK]);
  printf("Op Mobility:          "); printEvalFactor(Score.openingMobility[WHITE], Score.openingMobility[BLACK]);
  printf("eg Mobility:          "); printEvalFactor(Score.endgameMobility[WHITE], Score.endgameMobility[BLACK]);
  printf("Pawn structure:       "); printEvalFactor(Score.pawnStructure[WHITE], Score.pawnStructure[BLACK]);
  printf("Positional Metrics:   "); printEvalFactor(Score.positionMetrics[WHITE], Score.positionMetrics[BLACK]);
  printf("King Safety:          "); printEvalFactor(Score.kingSafety[WHITE], Score.kingSafety[BLACK]);
  printf("\n");
  printf("------------------------------------------\n");
}