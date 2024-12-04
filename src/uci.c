/*
Universal Chess Interface (UCI) is the standard communication protocol which GUIs use to communicate with chess engines.
More info on UCI: https://www.chessprogramming.org/UCI
*/

#include <string.h>
#include <stdlib.h>

#include "move.h"
#include "board.h"

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

// Parse position command from UCI
// Example commands: `position startpos`, `position fen <FEN_STRING>`
// The command can also include moves: `position startpos moves e2e4 e7e5`
void parse_position(char *command) {
    command += 9; // shift pointer to position argument
    char *current = command;

    // Parse startpos command and init board to start position
    if (strncmp(command, "startpos", 8) == 0) {
        load_fen(start_position);
    } else {
        current = strstr(command, "fen");

        if (current == NULL) {
            load_fen(start_position);
        } else {
            current += 4; // shift pointer to start of FEN string
            load_fen(current);
        }
    }

    current = strstr(command, "moves");

    // Parse moves
    if (current != NULL) {
         current += 6; // shift pointer to start of given moves token
         while (*current) {
            int move = parse_move(current);
            if (move == 0) {
                break;
            }
            make_move(move, ALL_MOVES);

            // Move pointer to next move
            while (*current && *current != ' ') {
                current++;   
            }
            current++;
         }
    }
}

// Parse the go command from UCI
// Example command: go depth 6
void parse_go(char * command) {
    int depth = -1;
    char *current = NULL;
    if (strstr(command, "depth")) {
        depth = atoi(current + 6); // shift pointer to start of depth
    } else {
        depth = 5; // TODO: placeholder
    }
}