#include "bitboard.h"

#define SOURCE 0x3f
#define TARGET 0xfc0
#define PIECE 0xf000
#define PROMOTED 0xf0000
#define CAPTURE 0x100000
#define DOUBLE 0x200000
#define ENPASSANT 0x400000
#define CASTLE 0x800000

#define encode_move(src, target, piece, promoted, capture, double, enpassant, castle) \
    (src) | (target << 6) | (piece << 12) | (promoted << 16) | (capture << 20) | (double << 21) | (enpassant << 22) | (castle << 23);

#define MOVE_SRC(move) (move & SOURCE);
#define MOVE_TARGET(move) ((move & TARGET) >> 6);
#define MOVE_PIECE(move) ((move & PIECE) >> 12);
#define MOVE_PROMOTED(move) ((move & PROMOTED) >> 16);
#define MOVE_CAPTURE(move) ((move & CAPTURE) >> 20);
#define MOVE_DOUBLE(move) ((move & DOUBLE) >> 21);
#define MOVE_ENPASSANT(move) ((move & ENPASSANT) >> 22);
#define MOVE_CASTLE(move) ((move & CASTLE) >> 23);

void generate_moves();
void generate_white_moves();
void generate_black_moves();

void generate_pawn_moves(int);
void generate_castling_moves(int);
void generate_knight_moves(int);
void generate_bishop_moves(int);
void generate_rook_moves(int);
void generate_queen_moves(int);
void generate_king_moves(int);