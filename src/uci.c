#include "move.h"

// Parse move string from UCI
// Example move from UCI protocol: "e7e8q"
int parse_move(const char *str) {
    Moves move_list[1];
    generate_moves(move_list);

    int src = (str[0] - 'a') + (8 - (str[1] - '0')) * 8;
    int target = (str[2] - 'a') + (8 - (str[3] - '0')) * 8;

    for (int i = 0; i < move_list->count; i++) {
        int move = move_list->moves[i];
        int move_src = MOVE_SRC(move); 
        int move_target = MOVE_TARGET(move);

        if (move_src == src && move_target == target) {
            int promoted_piece = MOVE_PROMOTED(move);
            if (promoted_piece) {
                char promotion_char = str[4];
                if ((promoted_piece == Q || promoted_piece == q) && promotion_char == 'q') return move;
                if ((promoted_piece == R || promoted_piece == r) && promotion_char == 'r') return move;
                if ((promoted_piece == B || promoted_piece == b) && promotion_char == 'b') return move;
                if ((promoted_piece == N || promoted_piece == n) && promotion_char == 'n') return move;
                continue;
            }
            return move;
        }
    }
    return 0;
}
