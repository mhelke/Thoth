#include "eval.h"
#include "bitboard.h"
#include "move.h"

#define MIRROR(square) ((square) ^ 56)
#define FILE_ABC_MASK 0x0707070707070707ULL
#define FILE_FGH_MASK 0xE0E0E0E0E0E0E0E0ULL

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
    -27,  -2,  -5,  12,  17,   6, 10, -25,
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
  -6,  -4,   1,   1,   1,   1,  -4,  -6,
  -6,  - 4,   1,   2,   2,   1,  -4,  -6,
  -6,  -4,   2,   8,   8,   2,  -4,  -6,
  -6,  -4,   5,  10,  10,   5,  -4,  -6,
  -4,  -4,   1,   5,   5,   1,  -4,  -4,
  -6,  -4,   1, -24,  -24,  1,  -4,  -6,
  0,   0,   0,   0,   0,   0,   0,   0
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
  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,
  -8,   0,   0,   0,   0,   0,   0,  -8,
  -8,   0,   4,   4,   4,   4,   0,  -8,
  -8,   0,   4,   8,   8,   4,   0,  -8,
  -8,   0,   4,   8,   8,   4,   0,  -8,
  -8,   0,   4,   4,   4,   4,   0,  -8,
  -8,   0,   1,   2,   2,   1,   0,  -8,
  -8, -12,  -8,  -8,  -8,  -8, -12,  -8
};

/***** Knights - Endgame *****
    The main principle in the endgame is mobility.
    Knights should aim for maximum mobility.
*******************/
static const int KNIGHT_ENDGAME_POSITION[64] = 
{
  -8,  -8,  -8,  -8,  -8,  -8,  -8,  -8,
  -8,   0,   0,   0,   0,   0,   0,  -8,
  -8,   0,   4,   4,   4,   4,   0,  -8,
  -8,   0,   4,   8,   8,   4,   0,  -8,
  -8,   0,   4,   8,   8,   4,   0,  -8,
  -8,   0,   4,   4,   4,   4,   0,  -8,
  -8,   0,   1,   2,   2,   1,   0,  -8,
  -8, -12,  -8,  -8,  -8,  -8, -12,  -8
};

/***** Bishops - Opening *****
     Since the endgame has fewer pieces, a more centralized knight is better.
     This is the only metric that can be used during this phase of the game given
     the importance of mobility.
*******************/
static const int BISHOP_OPENING_POSITION[64] = 
{
  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,
  -4,   0,   0,   0,   0,   0,   0,  -4,
  -4,   0,   2,   4,   4,   2,   0,  -4,
  -4,   0,   4,   6,   6,   4,   0,  -4,
  -4,   0,   4,   6,   6,   4,   0,  -4,
  -4,   1,   2,   4,   4,   2,   1,  -4,
  -4,   2,   1,   1,   1,   1,   2,  -4,
  -4,  -4, -12,  -4,  -4, -12,  -4,  -4
};

/***** Bishops - Endgame *****
    The main principle in the endgame is mobility.
    Bishops should aim for maximum mobility. 
*******************/
static const int BISHOP_ENDGAME_POSITION[64] = 
{
  -4,  -4,  -4,  -4,  -4,  -4,  -4,  -4,
  -4,   0,   0,   0,   0,   0,   0,  -4,
  -4,   0,   2,   4,   4,   2,   0,  -4,
  -4,   0,   4,   6,   6,   4,   0,  -4,
  -4,   0,   4,   6,   6,   4,   0,  -4,
  -4,   1,   2,   4,   4,   2,   1,  -4,
  -4,   2,   1,   1,   1,   1,   2,  -4,
  -4,  -4, -12,  -4,  -4, -12,  -4,  -4
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
  5,   5,   5,   5,   5,   5,   5,   5,
  20,  20,  20,  20,  20,  20,  20,  20,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  -5,   0,   0,   0,   0,   0,   0,  -5,
  0,   0,   0,   2,   2,   0,   0,   0

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
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 1, 1, 1, 0, 0,
  0, 0, 1, 2, 2, 1, 0, 0,
  0, 0, 2, 3, 3, 2, 0, 0,
  0, 0, 2, 3, 3, 2, 0, 0,
  0, 0, 1, 2, 2, 1, 0, 0,
  0, 0, 1, 1, 1, 1, 0, 0,
  -5, -5, -5, -5, -5, -5, -5, -5
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

// Material scores indexed by the piece type [P, N, B, R, Q, K, p, n, b, r, q, k]
static const int OPENING_MATERIAL_SCORE[12] = { 82, 337, 365, 477, 1025, 12000, -82, -337, -365, -477, -1025, -12000 };
static const int ENDGAME_MATERIAL_SCORE[12] = { 94, 281, 297, 512, 936, 12000, -94, -281, -297, -512,  -936, -12000 };

// Material scores adjusted based on the number of own pawns on the board.
// Rooks are better as pawns are traded off, while knights are worse.
static const int KNIGHT_ADJ[9] = { -20, -16, -12, -8, -4,  0,  4,  8, 12 };
static const int ROOK_ADJ[9] = { 15,  12,   9,  6,  3,  0, -3, -6, -9 };

static const int DOUBLE_PAWN_PENALTY = -10;
static const int ISOLATED_PAWN_PENALTY = -10;
static const int PASSED_PAWN_BONUS[8] = { 0, 10, 30, 50, 75, 100, 150, 200 }; 

static const int HALF_OPEN_FILE_SCORE = 5;
static const int OPEN_FILE_SCORE = 10;

static const int KING_SAFETY_BONUS = 5;
static const int BISHOP_PAIR_BONUS = 10;
static const int KNIGHT_PAIR_BONUS = 2;
static const int KNIGHT_PAIR_PENALTY = -3;
static const int BISHOP_ENDGAME_BONUS = 15;

// If the phase score is greater than this threshold, we are in the opening.
static const int OPENING_PHASE_THRESHOLD = 6192;

// If the phase score is less than this threshold, we are in the endgame.
static const int ENDGAME_PHASE_THRESHOLD = 518;

enum { OPENING, ENDGAME, MIDDLEGAME };

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

/*
    Game phase is determined by the major and minor pieces currently on the board.
    This value is used to determine not only what phase the game is in, but also
    how far along in the middle game the game is (see Tapered Evaluation comment below).
*/
static inline int calculate_game_phase_value(Board *board) {
    int white_pieces = 0, black_pieces = 0;
    
    for (int piece = N; piece <= Q; piece++) {
        white_pieces += count_bits(board->bitboards[piece]) * OPENING_MATERIAL_SCORE[piece];
    }
    for (int piece = n; piece <= q; piece++) {
        black_pieces += count_bits(board->bitboards[piece]) * -OPENING_MATERIAL_SCORE[piece];
    }

    return white_pieces + black_pieces;
}

static inline int interpolate(int opening_score, int endgame_score, int phase_value) {
    return (opening_score * phase_value + endgame_score * (OPENING_PHASE_THRESHOLD - phase_value)) / OPENING_PHASE_THRESHOLD;
}

int evaluate(Board *board) {
    int game_phase_value = calculate_game_phase_value(board);    
    int game_phase = -1;
    
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
    if (game_phase_value > opening_phase_threshold) {
        game_phase = OPENING;
    } else if (game_phase_value < ENDGAME_PHASE_THRESHOLD) {
        game_phase = ENDGAME;
    } else {
       game_phase = MIDDLEGAME;
    }

    /* 
        Static score keeps track of more complex positional evaluations such as pawn structure or king safety.
        Those values do not need to be interpolated as they are not dependent on the game phase.
        Opening/Endgame score keeps track of material and position and is calculated based on the game phase. 
        These values are used to interpolate the material and position scores in the middle game phase.
        The final evaluation score is a result of the static score and the respective game phase score (interpolated for middle game).
    */
    int static_score = 0, opening_score = 0, endgame_score = 0;
    int double_pawns;
    
    for (int piece = P; piece <= k; piece++) {
        Bitboard bitboard = board->bitboards[piece];
        while (bitboard) {
            int square = get_least_sig_bit_index(bitboard);
            int mirror_square = MIRROR(square);

            // Material
            opening_score += OPENING_MATERIAL_SCORE[piece];
            endgame_score += ENDGAME_MATERIAL_SCORE[piece];

            Bitboard file_mask = file_masks[square];

            int white_pawns_on_file = board->bitboards[P] & file_mask;
            int black_pawns_on_file = board->bitboards[p] & file_mask;
            int any_pawn_on_file = white_pawns_on_file | black_pawns_on_file;

            // Position
            switch (piece) {
                case P:
                    opening_score += PAWN_OPENING_POSITION[square];
                    endgame_score += PAWN_ENDGAME_POSITION[square];

                    double_pawns = count_bits(white_pawns_on_file);
                    if (double_pawns > 1) {
                        static_score += double_pawns * DOUBLE_PAWN_PENALTY;
                    }
                    if ((board->bitboards[P] & isolated_masks[square]) == 0) {
                        static_score += ISOLATED_PAWN_PENALTY;
                    }
                    if ((white_passed_masks[square] & board->bitboards[p]) == 0) {
                        static_score += PASSED_PAWN_BONUS[square_to_rank[square]];
                    }
                    break;
                case N: 
                    opening_score += KNIGHT_OPENING_POSITION[square];
                    endgame_score += KNIGHT_ENDGAME_POSITION[square];
                    break;
                case B: 
                    opening_score += BISHOP_OPENING_POSITION[square];
                    endgame_score += BISHOP_ENDGAME_POSITION[square];
                    static_score += count_bits(get_bishop_attacks(square, board->occupancies[BOTH], board));
                    break;
                case R:
                    opening_score += ROOK_OPENING_POSITION[square];
                    endgame_score += ROOK_ENDGAME_POSITION[square];

                    // Bonus for rooks on open files
                    if (white_pawns_on_file == 0) {
                       static_score += HALF_OPEN_FILE_SCORE;
                    }
                    if (any_pawn_on_file == 0) {
                       static_score += OPEN_FILE_SCORE;
                    }
                    static_score += count_bits(get_rook_attacks(square, board->occupancies[BOTH], board)); 
                    break;
                case Q:
                    opening_score += QUEEN_OPENING_POSITION[square];
                    endgame_score += QUEEN_ENDGAME_POSITION[square];
                    static_score += count_bits(get_queen_attacks(square, board->occupancies[BOTH], board)); 
                    break;
                case K:
                    opening_score += KING_OPENING_POSITION[square];
                    endgame_score += KING_ENDGAME_POSITION[square];
                    // Penalty for kings on exposed files
                    if (game_phase != ENDGAME) {
                        if (white_pawns_on_file == 0) {
                            static_score -= HALF_OPEN_FILE_SCORE;
                        }
                        if (any_pawn_on_file == 0) {
                        static_score -= OPEN_FILE_SCORE;
                        } 
                        // Pieces in front of king protecting it
                        static_score += count_bits(board->king_attacks[square] & board->occupancies[WHITE]) * KING_SAFETY_BONUS;
                    }
                    break;
                case p: 
                    opening_score -= PAWN_OPENING_POSITION[mirror_square];
                    endgame_score -= PAWN_ENDGAME_POSITION[mirror_square];
                    double_pawns = count_bits(black_pawns_on_file);
                    if (double_pawns > 1) {
                        static_score -= double_pawns * DOUBLE_PAWN_PENALTY;
                    }
                    
                    if ((board->bitboards[p] & isolated_masks[square]) == 0) {
                        static_score -= ISOLATED_PAWN_PENALTY;
                    }
                    
                    if ((black_passed_masks[square] & board->bitboards[P]) == 0) {
                        static_score -= PASSED_PAWN_BONUS[square_to_rank[mirror_square]];
                    }
                    break;
                case n:
                    opening_score -= KNIGHT_OPENING_POSITION[mirror_square];
                    endgame_score -= KNIGHT_ENDGAME_POSITION[mirror_square];
                    break;
                case b:
                    opening_score -= BISHOP_OPENING_POSITION[mirror_square]; 
                    endgame_score -= BISHOP_ENDGAME_POSITION[mirror_square]; 
                    static_score -= count_bits(get_bishop_attacks(square, board->occupancies[BOTH], board));
                    break;
                case r:
                    opening_score -= ROOK_OPENING_POSITION[mirror_square];
                    endgame_score -= ROOK_ENDGAME_POSITION[mirror_square];
                    // Bonus for rooks on open files
                    if (black_pawns_on_file == 0) {
                       static_score -= HALF_OPEN_FILE_SCORE;
                    }
                    if (any_pawn_on_file == 0) {
                       static_score -= OPEN_FILE_SCORE;
                    }
                    static_score -= count_bits(get_rook_attacks(square, board->occupancies[BOTH], board)); 
                    break;
                case q:
                    opening_score -= QUEEN_OPENING_POSITION[mirror_square];
                    endgame_score -= QUEEN_ENDGAME_POSITION[mirror_square];

                    static_score -= count_bits(get_queen_attacks(square, board->occupancies[BOTH], board)); 
                    break;
                case k:
                    opening_score -= KING_OPENING_POSITION[mirror_square];
                    endgame_score -= KING_ENDGAME_POSITION[mirror_square];
                    if (game_phase != ENDGAME) {
                        // Penalty for kings on exposed files
                        if (black_pawns_on_file == 0) {
                            static_score += HALF_OPEN_FILE_SCORE;
                        }
                        if (any_pawn_on_file == 0) {
                            static_score += OPEN_FILE_SCORE;
                        }  
                        // Pieces in front of king protecting it
                        static_score -= count_bits(board->king_attacks[square] & board->occupancies[BLACK]) * KING_SAFETY_BONUS;
                    }
                    break;
            }
            POP_BIT(bitboard, square);
        }
    }

    if (game_phase != OPENING) {
        int pawns = count_bits(board->bitboards[P] | board->bitboards[p]);
        int white_bishops = count_bits(board->bitboards[B]);
        int black_bishops = count_bits(board->bitboards[b]);
        int white_knights = count_bits(board->bitboards[N]);
        int black_knights = count_bits(board->bitboards[n]);
        
        // Bishop pair
        if (white_bishops > 1) static_score += BISHOP_PAIR_BONUS;
        if (black_bishops > 1) static_score -= BISHOP_PAIR_BONUS;

        // Knight pair
        if (count_bits(board->bitboards[N]) > 1) static_score += pawns > 10 ? KNIGHT_PAIR_BONUS : -KNIGHT_PAIR_PENALTY;
        if (count_bits(board->bitboards[n]) > 1) static_score -= pawns > 10 ? KNIGHT_PAIR_BONUS : -KNIGHT_PAIR_PENALTY;

        if (game_phase == ENDGAME) {
            // If there are pawns on both sides of the board, bishops are better than knights in the endgame
            int pawns_on_abc_files = (board->bitboards[P] & FILE_ABC_MASK) || (board->bitboards[p] & FILE_ABC_MASK);
            int pawns_on_fgh_files = (board->bitboards[P] & FILE_FGH_MASK) || (board->bitboards[p] & FILE_FGH_MASK);

            if (pawns_on_abc_files && pawns_on_fgh_files) {
                if (white_bishops > 0) {
                    static_score += BISHOP_ENDGAME_BONUS;
                }
                if (black_bishops > 0) {
                    static_score -= BISHOP_ENDGAME_BONUS;
                }
            }
        }
    }

    // Calculate final score
    int score = 0;
    switch (game_phase) {
        case OPENING: score = opening_score; break;
        case ENDGAME: score = endgame_score; break;
        case MIDDLEGAME: score = interpolate(opening_score, endgame_score, game_phase_value); break; 
    }
    score += static_score;
    return (board->side == WHITE) ? score : -score;
}